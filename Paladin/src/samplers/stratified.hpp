//
//  stratified.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/3.
//

#ifndef stratified_hpp
#define stratified_hpp

#include "core/header.h"
#include "core/sampler.hpp"

PALADIN_BEGIN

/**
 * 分层采样，基本思路很简单
 * 以一维变量为例，如果在[0,1)区间进行分层采样，n个样本。
 *
 * 1.先把区间平均分为n份，每个子区间我们成为一个cell
 * 2.以下要分两种情况，第一种是不添加样本扰动，直接采样每个子区间的中点
 *     第二种，添加样本扰动，随机采样每个子区间生成样本点
 * 3.对每个子区间的点进行随机重排列。
 *
 * 重排的目的主要是为了避免相邻维度的样本点都出自于同一个cell
 * 维度之间的样本点关联采样，生成的图像容易产生artifact
 */
class StratifiedSampler : public PixelSampler {
    
public:
    StratifiedSampler(int xPixelSamples, int yPixelSamples, bool jitterSamples,
                      int nSampledDimensions)
    : PixelSampler(xPixelSamples * yPixelSamples, nSampledDimensions),
    _xPixelSamples(xPixelSamples),
    _yPixelSamples(yPixelSamples),
    _jitterSamples(jitterSamples) {
        
    }
    
    virtual neb::CJsonObject toJson() const override;
    
    virtual void startPixel(const Point2i &) override;
    
    virtual std::unique_ptr<Sampler> clone(int seed) override;
    
private:

    const int _xPixelSamples, _yPixelSamples;
    
    const bool _jitterSamples;
};

Serialize_ptr createStratifiedSampler(const nebJson &param);

PALADIN_END

#endif /* stratified_hpp */
