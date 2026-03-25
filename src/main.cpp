#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>
#include <thread>
#include <chrono>

#if defined(__linux__)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <wayland-client.h>
#endif

#include "core/window.hpp"
#include "core/input.hpp"
#include "core/config.hpp"
#include "core/logger.hpp"
#include "renderer/renderer.hpp"
#include "game/game.hpp"

int main(int argc, char* argv[]) {
    viber::Logger::init();
    
    auto logger = spdlog::get("viber");
    logger->info("ViberRacing starting up...");
    
    try {
        viber::Config config;
        config.loadFromFile("assets/config/default.json");
        
        viber::WindowConfig windowConfig;
        windowConfig.width = config.getInt("window.width", 1280);
        windowConfig.height = config.getInt("window.height", 720);
        windowConfig.title = config.getString("window.title", "ViberRacing");
        windowConfig.fullscreen = config.getBool("window.fullscreen", false);
        windowConfig.vsync = config.getBool("window.vsync", true);
        
        viber::Window window(windowConfig);
        logger->info("Window created");
        
        // Process events to let the compositor configure the window
        // Wayland needs a few event loops before the surface is ready
        for (int i = 0; i < 30; i++) {
            window.pollEvents();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        viber::Input input;
        logger->info("Input created");
        
        viber::Renderer renderer(window);
        logger->info("Renderer initialized");
        
        viber::Game game(window, input, renderer, config);
        logger->info("Game created");
        
        game.init();
        logger->info("Game initialized");
        
        // Give Wayland compositor time to map the window
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        logger->info("Entering main loop");
        
        while (!window.shouldClose()) {
            // Poll events multiple times for Wayland compatibility
            for (int i = 0; i < 3; i++) {
                window.pollEvents();
            }
            
#if defined(__linux__)
            // Dispatch Wayland events to ensure compositor communication
            if (window.isWayland()) {
                struct wl_display* display = (struct wl_display*)glfwGetWaylandDisplay();
                if (display) {
                    wl_display_dispatch_pending(display);
                }
            }
#endif
            
            input.update();
            
            game.update(1.0f / 60.0f);
            game.render();
            // Note: No swapBuffers here - bgfx handles rendering and presentation
        }
        
        logger->info("Shutting down...");
        
    } catch (const std::exception& e) {
        logger->error("Fatal error: {}", e.what());
        return 1;
    }
    
    return 0;
}
