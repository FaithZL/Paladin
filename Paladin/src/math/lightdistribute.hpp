//
//  lightdistribute.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/30.
//

#ifndef lightdistribute_hpp
#define lightdistribute_hpp


#include "sampling.hpp"
#include "core/scene.hpp"

PALADIN_BEGIN

class LightDistribution {
    
public:
    LightDistribution() {
        
    }
    
    virtual const Distribution1D * lookup(const Point3f &p) const = 0;
};

std::unique_ptr<Distribution1D> computeLightPowerDistribution(const Scene &scene);

class UniformLightDistribution : public LightDistribution {
public:
    UniformLightDistribution(const Scene &scene);
    
    
    virtual const Distribution1D * lookup(const Point3f &p) const;
private:
    
    std::unique_ptr<Distribution1D> _distribution;
};

class PowerLightDistribution : public LightDistribution {
public:
    PowerLightDistribution(const Scene &scene);
    
    virtual const Distribution1D * lookup(const Point3f &p) const;
private:
    std::unique_ptr<Distribution1D> _distribution;
};

std::unique_ptr<LightDistribution> createLightSampleDistribution(
                                                                 const std::string &name, const Scene &scene);

PALADIN_END

#endif /* lightdistribute_hpp */
