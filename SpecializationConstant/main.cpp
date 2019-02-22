#include"Demo.h"

//debug test----------------------------------
#include"MxUtils.h"
void drawSlipLine() {
    std::cout << "------------------------" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec3) {
    return os << "[ " << vec3.x << ", " << vec3.y << ", " << vec3.z << " ]";
}

std::ostream& operator<<(std::ostream& os, const Mix::Axis& axis) {
    return os << "{ " << axis.x << ", " << axis.y << ", " << axis.z << " }";
}

std::ostream& operator<<(std::ostream& os, const glm::quat& quat) {
    return os << "[ " << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << " ]";
}

std::ostream& operator<<(std::ostream& os, const Mix::Transform& trans) {
    std::cout << trans.getPosition() << std::endl;
    std::cout << trans.getAxis() << std::endl;
    return std::cout << trans.getEulerAngles() << std::endl;
}
//debug test----------------------------------
int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error : Failed to initialize SDL2!" << std::endl;
        return -1;
    }

    TestDemo demo;
    demo.init();
    demo.run();
    demo.destroy();
    return 0;
}
