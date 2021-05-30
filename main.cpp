#define SDL_MAIN_HANDLED

#include <init.h>

int main() {
    mb::init instance;
    instance.littleLoop();
    return 0;
}
