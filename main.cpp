#define SDL_MAIN_HANDLED

#include <init.h>

int main() {
    mb::init instance;
    mb::init::littleLoop();
    return 0;
}