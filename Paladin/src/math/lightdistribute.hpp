//
//  lightdistribute.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/30.
//

#ifndef lightdistribute_hpp
#define lightdistribute_hpp


#include "sampling.hpp"

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
    
    std::unique_ptr<Distribution1D> _distribute;
};

class PowerLightDistribution : public LightDistribution {
public:
    PowerLightDistribution(const Scene &scene);
    
    virtual const Distribution1D * lookup(const Point3f &p) const;
private:
    std::unique_ptr<Distribution1D> _distribute;
};

PALADIN_END

#endif /* lightdistribute_hpp */
