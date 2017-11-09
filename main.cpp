//COMP3501 Assignment 2 - Matthew Moulton (101010631) - October 13, 2017
//Default controls of the demo were kept, with the additions of "W" to stop all momentum (a brake), and "E" to fire a laser.

#include <iostream>
#include <exception>
#include "game.h"

// Macro for printing exceptions
#define PrintException(exception_object)\
	std::cerr << exception_object.what() << std::endl

// Main function that builds and runs the game
int main(void){
    game::Game app; // Game application

    try {
        // Initialize game
        app.Init();
        // Setup the main resources and scene in the game
        app.SetupResources();
        app.SetupScene();
        // Run game
        app.MainLoop();
    }
    catch (std::exception &e){
        PrintException(e);
    }

    return 0;
}
