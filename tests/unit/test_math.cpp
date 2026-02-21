#include <catch2/catch_test_macros.hpp>
#include "utils/math.hpp"
#include <cmath>

TEST_CASE("Math utilities", "[math]") {
    SECTION("lerp") {
        REQUIRE(viber::lerp(vec3(0.0f), vec3(10.0f), 0.5f) == vec3(5.0f));
        REQUIRE(viber::lerp(vec3(-5.0f), vec3(5.0f), 0.0f) == vec3(-5.0f));
        REQUIRE(viber::lerp(vec3(-5.0f), vec3(5.0f), 1.0f) == vec3(5.0f));
    }
    
    SECTION("clamp01") {
        REQUIRE(viber::clamp01(-0.5f) == 0.0f);
        REQUIRE(viber::clamp01(0.5f) == 0.5f);
        REQUIRE(viber::clamp01(1.5f) == 1.0f);
    }
    
    SECTION("clampMagnitude") {
        vec3 v(10.0f, 0.0f, 0.0f);
        vec3 clamped = viber::clampMagnitude(v, 5.0f);
        REQUIRE(glm::length(clamped) == Approx(5.0f));
        
        vec3 v2(3.0f, 0.0f, 0.0f);
        vec3 clamped2 = viber::clampMagnitude(v2, 5.0f);
        REQUIRE(glm::length(clamped2) == Approx(3.0f));
    }
    
    SECTION("smoothstep") {
        REQUIRE(viber::smoothstep(0.0f, 1.0f, 0.0f) == Approx(0.0f));
        REQUIRE(viber::smoothstep(0.0f, 1.0f, 1.0f) == Approx(1.0f));
        REQUIRE(viber::smoothstep(0.0f, 1.0f, 0.5f) == Approx(0.5f));
    }
    
    SECTION("reflect") {
        vec3 v(1.0f, -1.0f, 0.0f);
        vec3 n(0.0f, 1.0f, 0.0f);
        vec3 r = viber::reflect(v, n);
        REQUIRE(r.x == Approx(1.0f));
        REQUIRE(r.y == Approx(1.0f));
        REQUIRE(r.z == Approx(0.0f));
    }
    
    SECTION("isFinite") {
        REQUIRE(viber::isFinite(1.0f));
        REQUIRE_FALSE(viber::isFinite(std::numeric_limits<float>::infinity()));
        REQUIRE_FALSE(viber::isFinite(std::numeric_limits<float>::quiet_NaN()));
        
        REQUIRE(viber::isFinite(vec3(1.0f, 2.0f, 3.0f)));
        REQUIRE_FALSE(viber::isFinite(vec3(1.0f, std::numeric_limits<float>::infinity(), 3.0f)));
    }
}
