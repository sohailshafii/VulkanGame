
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "GameApplicationLogic.h"

int main() {
	// in case we use rand anywhere, set up seed here
	srand((unsigned int)time(NULL));

	GameApplicationLogic app;

	try {
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
