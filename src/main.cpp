#include "gameengine.hpp"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    //Main, no need to touch
    GameEngine g("./config.txt");
    g.run();

    return 0;
}