
#include <SFML/Graphics.hpp>
#include "Game.h"



int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	
	Game g("config.txt");
	g.run();
	return  0;
}
