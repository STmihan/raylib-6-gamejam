#include "app/entrypoint.h"

int main(void) {
#if defined(__EMSCRIPTEN__)
    return app::RunWeb();
#else
    return app::RunDesktop();
#endif
}
