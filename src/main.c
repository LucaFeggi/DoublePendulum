#include "app/app.h"

#include <stdlib.h>

int main(void) {
    App app;

    if(!app_init(&app)) {
        return EXIT_FAILURE;
    }

    app_run(&app);
    app_quit(&app);

    return EXIT_SUCCESS;
}
