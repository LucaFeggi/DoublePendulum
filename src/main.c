#include "app/app.h"

int main(){
    App app;
    if(app_init(&app)){
        app_run(&app);
        app_quit(&app);
    }
    return 0;
}