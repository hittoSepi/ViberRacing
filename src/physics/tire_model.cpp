#include "tire_model.hpp"
#include <cmath>

namespace viber {

TireModel::TireModel() {
    m_params = {};
}

void TireModel::setParams(const Params& params) {
    m_params = params;
}

float TireModel::calculateGrip(float slipRatio, float slipAngle, float normalForce) {
    float longGrip = calculateLongitudinalGrip(slipRatio);
    float latGrip = calculateLateralGrip(slipAngle);
    
    float combinedGrip = calculateCombinedGrip(longGrip, latGrip);
    
    return applyLoadSensitivity(combinedGrip, normalForce);
}

float TireModel::calculateLongitudinalGrip(float slipRatio) {
    return pacejkaMagicFormula(
        slipRatio,
        m_params.peakSlipRatio,
        m_params.peakGrip,
        m_params.asymptoticGrip,
        m_params.slipRatioAtAsymptote
    );
}

float TireModel::calculateLateralGrip(float slipAngle) {
    float slipAngleRad = slipAngle * DEG_TO_RAD;
    
    return pacejkaMagicFormula(
        slipAngleRad,
        m_params.peakSlipAngle * DEG_TO_RAD,
        m_params.peakLateralGrip,
        m_params.asymptoticLateralGrip,
        m_params.slipAngleAtAsymptote * DEG_TO_RAD
    );
}

float TireModel::calculateCombinedGrip(float longGrip, float latGrip) {
    float combinedSquared = longGrip * longGrip + latGrip * latGrip;
    float peakGrip = std::max(m_params.peakGrip, m_params.peakLateralGrip);
    
    if (combinedSquared > peakGrip * peakGrip) {
        return peakGrip;
    }
    
    return std::sqrt(combinedSquared);
}

float TireModel::pacejkaMagicFormula(float slip, float peakSlip, float peakGrip,
                                      float asymptoticGrip, float slipAtAsymptote) {
    float absSlip = std::abs(slip);
    
    if (absSlip < 0.0001f) {
        return 0.0f;
    }
    
    float normalizedSlip = absSlip / peakSlip;
    
    float x = normalizedSlip;
    float x2 = x * x;
    float x3 = x2 * x;
    
    float B = 10.0f;
    float C = 1.9f;
    float D = peakGrip;
    float E = 0.97f;
    
    float grip = D * std::sin(C * std::atan(B * x - E * (B * x - std::atan(B * x))));
    
    if (absSlip > slipAtAsymptote) {
        float t = (absSlip - slipAtAsymptote) / slipAtAsymptote;
        t = std::min(t, 1.0f);
        float asymptoticValue = asymptoticGrip;
        grip = grip * (1.0f - t) + asymptoticValue * t;
    }
    
    return std::max(grip, 0.0f);
}

float TireModel::applyLoadSensitivity(float baseGrip, float normalForce) {
    float loadRatio = normalForce / m_params.referenceLoad;
    float sensitivity = m_params.loadSensitivity * (loadRatio - 1.0f);
    
    return baseGrip * (1.0f - sensitivity);
}

ArcadeTireModel::ArcadeTireModel() 
    : TireModel() {
    m_gripMultiplier = 1.2f;
}

float ArcadeTireModel::calculateGrip(float slipRatio, float slipAngle, float normalForce) {
    float absSlip = std::abs(slipRatio) + std::abs(slipAngle * DEG_TO_RAD);
    
    if (absSlip < 0.01f) {
        return m_gripMultiplier;
    }
    
    float grip = m_gripMultiplier;
    
    if (absSlip > m_slipThreshold) {
        float excessSlip = absSlip - m_slipThreshold;
        float penalty = 1.0f - excessSlip * 0.5f;
        penalty = std::max(penalty, 0.5f);
        grip *= penalty;
    }
    
    float loadFactor = 1.0f + (normalForce / 5000.0f - 1.0f) * 0.1f;
    grip *= loadFactor;
    
    return grip;
}

}
