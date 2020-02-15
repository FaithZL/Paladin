//
//  fourierbsdf.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/15.
//

#ifndef fourierbsdf_hpp
#define fourierbsdf_hpp

#include "core/bxdf.hpp"

PALADIN_BEGIN

/**
 * 接下来介绍一下傅里叶BSDF
 * 参考章节
 * http://www.pbr-book.org/3ed-2018/Reflection_Models/Fourier_Basis_BSDFs.html
 *
 * 虽然像Torrance-Sparrow和Oren-Nayar这样的反射模型可以准确地表示许多材料，
 * 但是有些材料的BRDF形状与这些模型并不匹配。
 * (例如金属层状材料，表面有光滑或粗糙的涂层或织物，这些材料通常会有部分反光。)
 * 这类材料的一种选择是将它们的BSDF值存储在一个大型3D或4D查找表中，但这种方法可能需要非常大的存储空间
 * 例如，如果要采样 ωi, ωo两个方向，每个方向包含两个维度θ与φ，如果每个样本间隔为一度，
 * 需要的储存空间是相当巨大的，大概为(180 * 360)^2个样本
 *
 * 因此，更紧凑的表示并能够准确地表示BSDF才是最好的办法
 * 接下来将介绍FourierBSDF，它使用傅里叶基表示带有余弦项和的BSDFs。
 * 这种表示方法不仅节省空间，还能精确的表达，并且能让蒙特卡洛积分很好的工作
 *
 * 在这里，我们不讨论如何将BSDFs转换为这种表示形式，
 * 但是我们将重点讨论它在渲染中的使用。
 * FourierBSDF表示各向同性BSDF，通过一对球坐标对BSDF进行参数化，分别表示入射方向和出射方向，
 * 可以表示为
 *
 *          f(ωi, ωo) = f(μi, φi, μo, φo)
 *
 * 其中μi = cosθi, μo = cosθo, 我们假设该bsdf为各向同性，则可以表示为φ = φi - φo
 *
 *          f(ωi, ωo) = f(μi, φi, μo, φo) = f(μi, μo, φ)
 *
 * 各向同性BSDF通常也是关于方位角差的偶函数
 *
 *           f(μi, μo, φ) = f(μi, μo, -φ)
 *
 * 根据这些性质，BSDF与余弦衰减的乘积在方位角差中被表示为傅立叶级数的形式
 *
 *                          m-1
 * f(μi, μo, φi - φo)|μi| =  ∑ ak(μi, μo) cos(k(φi - φo))    8.21
 *                          k=0
 *
 * 咦？怎么只有cos项，sin项去哪里了？？？  都说了这是偶函数嘛，sin项没了
 * _
 * μ = {μ0,μ1,μ2....μ_n-1} , 0 < i,j < n ,
 *
 * ak(μi,μj) 理解为是一个n x n的矩阵，所以一个brdf函数可以理解为由这样一个矩阵组成
 *
 * 计算公式(8.21)以达到令人满意的精度所需的最大阶数是变化的:
 * 它取决于特定的天顶角，因此对于给定的一对方向，必须根据BSDF的复杂性调整系数的数量。
 * 这样做对于表示的紧凑性非常重要
 *
 * 要查看能够改变系数数量的值，考虑接近理想镜面反射μi ≈ μo，要准确地表示镜面波瓣需要许多系数
 * 当φi - φo = π，brdf函数值非常大，入射方向跟出射方向几乎相反，但μi与μo不是对称的。
 * 只需要一项就可以表示BSDF为0(或具有可忽略的值)
 *
 * 平滑BSDF,大多数或所有μi和μo角度对需要多个系数ak准确地代表他们的φ分布,
 * 但是它们的平滑性意味着对于每个ak通常不需要太多的系数。
 * FourierBSDF表示法利用了这一特性，并且只存储实现所需精度所需的稀疏系数集
 * 因此，对于大多数类型的实际BSDF数据，式(8.21)的表示相当紧凑;典型的是几兆字节
 *
 *
 * FourierBSDFTable结构提供了相关数据以及方法
 *
 *
 * 为了估计8.21式我们需要知道阶数m，以及对应ωi,ωo的所有系数a0,a1,a2....a_m-1
 * 为简单起见,我们只提供基本思路如下，最近的系数μ方向小于或等于μi和μo,
 * 尽管实现遵循间作插值系数从多个方向μ值。（todo，这里暂时不理解）
 *
 * 傅里叶表达式中的阶数m始终受限于mMax,
 * 但随入射方向与出射方向的天顶角余弦值μi和μo的变化而变化
 * m的值为多少？可以通过查询一个nMu × nMu整数矩阵mat获取到
 *
 * 为了找到特定的方向，ωi,ωo所对应的阶数m，我们可以用二分法在mu列表中查找
 *       mu[i] <= μi < mu[i + 1]
 *       mu[o] <= μo < mu[o + 1]
 * 用以上两个索引i,o可以在mMat矩阵中查询到m值，m = mMat[o * nMu + i]
 *
 * 所有关于离散方向对的系数ak被打包储存到一个数组a中，
 * 因为最大的阶数(系数的数量)是变化的，甚至可以是零，
 * 这取决于给定方向对的BSDF的特性，所以找到数组a中的偏移量是一个两步的过程:
 *
 *    1.偏移量i与o通过一个关系式映射到一个数组aOffset中，这个数组储存着
 *      数组a的偏移量， offset = aOffset[o * nMu + i] (aOffset的尺寸也是nMu^2)
 *    2.从a[offset]开始的m个元素，确定了指定方向对所对应的m个系数(也就是a0,a1...a_m-1)
 *      如果是三个通道，则第一个m表示亮度通道，第二个表示红色通道，第三个表示绿色通道
 *
 * 为了使重建BSDF更加平滑ak需要用插值计算
 *
 *      3   3
 * ak = ∑   ∑ ak(i+a, o+b)wi(a)wo(b)
 *     a=0 b=0
 *
 *
 *
 *
 *
 *
 */

/**
 *
 * 为了简单起见，我们现在给出的基本思想就好像是最接近的方向的系数小于等于
 *
 *
 */
struct FourierBSDFTable {
    // 折射率
    Float eta;
    // 8.21式中最大阶数
    int mMax;
    // 通道数，通常为1或3
    // 为1时，表示单色bsdf
    // 为3时，三个通道分别为亮度，红，蓝，三个通道
    // 直接表示亮度对于蒙特卡洛采样非常有用，并且用这三个量也容易计算出绿色通道
    int nChannels;

    // 将天顶角离散成nMu个方向，储存在mu中
    // μ的数量
    int nMu;
    // μ列表，从小到大排列，尺寸为nMu * nMu
    Float *mu;

    // nMu × nMu矩阵，储存对应的阶数m
    int *mMat;

    // 系数ak列表
    Float *a;
    // ak列表的偏移，尺寸为nMu * nMu的列表
    int *aOffset;
    Float *a0;
    Float *cdf;
    // 1/i
    Float *recip;

    static bool read(const std::string &filename, FourierBSDFTable *table);

    const Float *getAk(int offsetI, int offsetO, int *mptr) const {
        *mptr = mMat[offsetO * nMu + offsetI];
        return a + aOffset[offsetO * nMu + offsetI];
    }

    bool getWeightsAndOffset(Float cosTheta, int *offset,
                             Float weights[4]) const;
};

class FourierBSDF : public BxDF {

public:

    FourierBSDF(const FourierBSDFTable &bsdfTable, TransportMode mode)
    :BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY)),
    _bsdfTable(bsdfTable),
    _mode(mode) {

    }

    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override;

    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                  Float *pdf, BxDFType *sampledType) const override;

    virtual Float pdfDir(const Vector3f &wo, const Vector3f &wi) const override;

    virtual std::string toString() const override;
    
private:
    const FourierBSDFTable &_bsdfTable;
    
    const TransportMode _mode;
};

PALADIN_END

#endif /* fourierbsdf_hpp */
