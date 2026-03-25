#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "utils/spline.hpp"

using Catch::Approx;
#include <cmath>

TEST_CASE("Spline", "[spline]") {
    viber::Spline spline;
    
    SECTION("Empty spline") {
        REQUIRE(spline.getNumControlPoints() == 0);
        REQUIRE(spline.evaluate(0.0f) == viber::vec3(0.0f));
    }
    
    SECTION("Add control points") {
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(10.0f, 0.0f, 0.0f));
        
        REQUIRE(spline.getNumControlPoints() == 2);
        
        auto points = spline.getControlPoints();
        REQUIRE(points[0] == viber::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(points[1] == viber::vec3(10.0f, 0.0f, 0.0f));
    }
    
    SECTION("Evaluate spline") {
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(10.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(20.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(30.0f, 0.0f, 0.0f));
        
        viber::vec3 p0 = spline.evaluate(0.0f);
        viber::vec3 p1 = spline.evaluate(1.0f);
        
        REQUIRE(p0.x == Approx(0.0f).margin(0.1f));
        REQUIRE(p1.x > 20.0f);
    }
    
    SECTION("Get tangent") {
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 10.0f));
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 20.0f));
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 30.0f));
        
        viber::vec3 tangent = spline.getTangent(0.5f);
        REQUIRE(tangent.z > 0.0f);
    }
    
    SECTION("Closed loop") {
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(10.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(10.0f, 0.0f, 10.0f));
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 10.0f));
        
        spline.closeLoop();
        REQUIRE(spline.isClosedLoop());
    }
    
    SECTION("Sample points") {
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(10.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(20.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(30.0f, 0.0f, 0.0f));
        
        auto samples = spline.sample(10);
        REQUIRE(samples.size() == 10);
    }
    
    SECTION("Get frame") {
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(10.0f, 0.0f, 10.0f));
        spline.addControlPoint(viber::vec3(20.0f, 0.0f, 0.0f));
        spline.addControlPoint(viber::vec3(30.0f, 0.0f, 10.0f));
        
        auto frame = spline.getFrame(0.5f);
        
        float tangentLen = glm::length(frame.tangent);
        float normalLen = glm::length(frame.normal);
        float binormalLen = glm::length(frame.binormal);
        
        REQUIRE(tangentLen == Approx(1.0f).margin(0.01f));
        REQUIRE(normalLen == Approx(1.0f).margin(0.01f));
        REQUIRE(binormalLen == Approx(1.0f).margin(0.01f));
    }
}
