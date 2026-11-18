#include "sdl_application.hpp"

#if defined(__ANDROID__)
#include <SDL3/SDL_main.h>
#endif

#if defined(__ANDROID__)
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
#else
int main() {
#endif
    pd::platform::sdl::SdlApplication application;
    return application.run();
}
