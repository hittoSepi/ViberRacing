#pragma once

#include "core/types.hpp"

namespace viber {

class TireModel {
public:
    TireModel();
    virtual ~TireModel() = default;
    
    struct Params {
        float peakSlipRatio = 0.1f;
        float peakGrip = 1.8f;
        float asymptoticGrip = 1.2f;
        float slipRatioAtAsymptote = 0.4f;
        
        float peakSlipAngle = 8.0f;
        float peakLateralGrip = 1.5f;
        float asymptoticLateralGrip = 1.0f;
        float slipAngleAtAsymptote = 25.0f;
        
        float loadSensitivity = 0.01f;
        float referenceLoad = 4000.0f;
    };
    
    void setParams(const Params& params);
    const Params& getParams() const { return m_params; }
    
    virtual float calculateGrip(float slipRatio, float slipAngle, float normalForce);
    
    float calculateLongitudinalGrip(float slipRatio);
    float calculateLateralGrip(float slipAngle);
    
    float calculateCombinedGrip(float longGrip, float latGrip);
    
protected:
    float pacejkaMagicFormula(float slip, float peakSlip, float peakGrip, 
                               float asymptoticGrip, float slipAtAsymptote);
    
    float applyLoadSensitivity(float baseGrip, float normalForce);
    
private:
    Params m_params;
};

class ArcadeTireModel : public TireModel {
public:
    ArcadeTireModel();
    
    float calculateGrip(float slipRatio, float slipAngle, float normalForce) override;
    
    void setGripMultiplier(float multiplier) { m_gripMultiplier = multiplier; }
    float getGripMultiplier() const { return m_gripMultiplier; }
    
private:
    float m_gripMultiplier = 1.0f;
    float m_slipThreshold = 0.15f;
};

}
