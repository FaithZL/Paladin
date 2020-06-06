//
//  material.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef material_hpp
#define material_hpp

#include "core/header.h"
#include "cobject.h"
#include "tools/classfactory.hpp"

PALADIN_BEGIN



/**
 * 材质基类主要实现的是bump函数
 */
class Material : public CObject {
public:
    
    Material(const shared_ptr<Texture<Spectrum>> &normalMap = nullptr,
             const shared_ptr<Texture<Float>> &bumpMap = nullptr,
             Float scale = -1);
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
    
    virtual ~Material() {
    	
    }
    
    void updateSi(const SurfaceInteraction &si);
    
    inline void processNormal(SurfaceInteraction * si) const {
        if (_normalMap) {
            normalMapping(_normalMap, si, _normalMapScale);
        } else if (_bumpMap) {
            bumpMapping(_bumpMap, si);
        }
    }
    
    static void normalMapping(const shared_ptr<Texture<Spectrum>> &normalMap,
                              SurfaceInteraction * si, Float scale = -1);

    /**
     * bump函数，用于更新SurfaceInteraction变量
     * 计算对应点的偏移值并更新
     * 简述一下算法
     * 假设点p经过bump偏移后的点为 p', n(u,v)为uv坐标点的法向量，b(u,v)为偏移函数
     * 
     * 则 p‘(u,v) = p(u,v) + n(u,v) b(u,v)  1式
     *
     * 1式用人话来说就是p点经过bump贴图映射后的坐标为原始坐标加上bump偏移乘以法向量
     * 
     * p'(u,v)对于u的偏导数，直接带入1式，用乘法求导法则展开
     *
     *  dp'(u,v)    dp(u,v)     db(u,v)                  dn(u,v)
     * --------- = --------- + -------- n(u,v) + b(u,v) ---------
     *     du         du          du                       du
     *
     * dp(u,v)/du我们在surfaceinteraction中已经计算过了
     * dn(u,v)/du，n(u,v)也已经在计算好了，就还差个 db(u,v)/du
     *
     * 有两种方式可以近似 db(u,v)/du 项
     * 1.用相邻两个纹理像素的值来计算偏导数，但这种方式很难适扩展到复杂的过程纹理
     * 
     * 因此，我们打算用正向差分的方式去直接计算偏导数
     *
     * 回顾导数的定义
     *
     *  db(u,v)               b(u + △u, v) - b(u, v)
     * --------- = lim[△u→0]--------------------------
     *    du                          △u
     *
     * 用有限小量去近似
     * 
     *  dp'(u,v)	dp(u,v)	   db(u+△u,v) - db(u,v)	                dn(u,v)
     * --------- ≈ --------- + --------------------- n(u,v) + b(u,v) ------
     *     du		  du                △u                             du
     *
     * 大多数纹理的实现都忽略最后一项，因为b(u,v)本来就是一个微小的偏移，所以这个方式很合理
     * 很多渲染器是不计算dn/du, dn/dv，可能是因为做了上述的近似
     * 忽略最后一项的原因是偏移量不会影响偏导数的大小
     * 但我们会把三项都计算在内，因为dn/du已经有了
     * 
     * 
     * todo这里我暂时没理解上面所说的过程纹理
     *     
     * @param d  [description]
     * @param si [description]
     */
    static void bumpMapping(const std::shared_ptr<Texture<Float>> &d,
                     SurfaceInteraction *si);
    
    shared_ptr<BSDF> _bsdf;
    
protected:
    // 法线贴图
    std::shared_ptr<Texture<Spectrum>> _normalMap;
    // bump贴图
    std::shared_ptr<Texture<Float>> _bumpMap;
    // [-1,1]
    Float _normalMapScale;
    

};

Material * createMaterial(const nloJson &);

PALADIN_END

#endif /* material_hpp */
