
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "GameApplicationLogic.h"
#include "FreeTypeInterface.h"

int main() {
	// in case we use rand anywhere, set up seed here
	srand(time(NULL));

	GameApplicationLogic app;

	// just test the font interface here for now
	FreeTypeInterface freeTypeInterface;

	try {
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
