#include <catch2/catch_test_macros.hpp>
#include "physics/world.hpp"
#include "physics/vehicle.hpp"

TEST_CASE("PhysicsWorld", "[physics]") {
    viber::PhysicsWorld world;
    
    SECTION("Initialization") {
        REQUIRE(world.getGravity() == viber::vec3(0.0f, -9.81f, 0.0f));
    }
    
    SECTION("Set gravity") {
        world.setGravity(viber::vec3(0.0f, -20.0f, 0.0f));
        REQUIRE(world.getGravity() == viber::vec3(0.0f, -20.0f, 0.0f));
    }
    
    SECTION("Step simulation") {
        REQUIRE_NOTHROW(world.step(1.0f / 60.0f));
    }
    
    SECTION("Raycast - no hit") {
        viber::vec3 hitPoint, hitNormal;
        bool hit = world.raycast(
            viber::vec3(0.0f, 10.0f, 0.0f),
            viber::vec3(0.0f, 0.0f, 0.0f),
            hitPoint, hitNormal
        );
        REQUIRE_FALSE(hit);
    }
}

TEST_CASE("Vehicle", "[physics]") {
    viber::PhysicsWorld world;
    viber::Vehicle vehicle;
    
    SECTION("Create vehicle") {
        REQUIRE(vehicle.create(&world));
        REQUIRE(vehicle.getBulletVehicle() != nullptr);
    }
    
    SECTION("Vehicle position") {
        vehicle.create(&world);
        vehicle.setPosition(viber::vec3(10.0f, 5.0f, 20.0f));
        
        viber::vec3 pos = vehicle.getPosition();
        REQUIRE(pos.x == Approx(10.0f));
        REQUIRE(pos.y == Approx(5.0f));
        REQUIRE(pos.z == Approx(20.0f));
    }
    
    SECTION("Reset vehicle") {
        vehicle.create(&world);
        vehicle.reset(viber::vec3(0.0f, 1.0f, 0.0f), viber::vec3(0.0f, 0.0f, 1.0f));
        
        viber::vec3 forward = vehicle.getForward();
        REQUIRE(forward.z > 0.5f);
    }
    
    SECTION("Controls") {
        vehicle.create(&world);
        
        REQUIRE_NOTHROW(vehicle.setThrottle(1.0f));
        REQUIRE_NOTHROW(vehicle.setBrake(1.0f));
        REQUIRE_NOTHROW(vehicle.setSteering(-1.0f));
    }
}
