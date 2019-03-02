#include"Demo.h"

#include<vulkan/vulkan.hpp>
int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error : Failed to initialize SDL2!" << std::endl;
        return -1;
    }

    TestDemo demo;
    try {
        demo.init();
        demo.run();
        demo.destroy();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::ends << std::endl;
    }
    return 0;
}
