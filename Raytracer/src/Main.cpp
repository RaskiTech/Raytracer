#include <iostream>
#include "App.h"

// Don't try to steal my main, SDL
#undef main

int main()
{
    App* app = new App();
    app->Loop();

    return 0;
}