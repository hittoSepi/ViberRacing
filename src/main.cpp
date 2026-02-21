#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>

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
        
        viber::Window window(config);
        viber::Input input;
        viber::Renderer renderer(window);
        
        viber::Game game(window, input, renderer, config);
        
        logger->info("Entering main loop");
        
        while (!window.shouldClose()) {
            window.pollEvents();
            input.update();
            
            game.update(1.0f / 60.0f);
            game.render();
            
            window.swapBuffers();
        }
        
        logger->info("Shutting down...");
        
    } catch (const std::exception& e) {
        logger->error("Fatal error: {}", e.what());
        return 1;
    }
    
    return 0;
}
