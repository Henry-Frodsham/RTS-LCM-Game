//Copyright © 2025 Henry Frodsham
#include "Application.h"
#include <doctest/doctest.h>

// startup function, delegates responsibility to the application class
int main(int argc, char** argv) {
    Application* MainApp = new Application();

    MainApp->Start();

    // shutdown
    delete MainApp;
    return 0;
}