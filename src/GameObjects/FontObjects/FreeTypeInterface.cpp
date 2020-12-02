#include "FreeTypeInterface.h"
#include <iostream>

FreeTypeInterface::FreeTypeInterface() {
	if (FT_Init_FreeType(&freeTypeLib)) {
		std::cout << "Could not initialize the FreeType library.\n";
		return;
	}
}

FreeTypeInterface::~FreeTypeInterface() {
	FT_Done_FreeType(freeTypeLib);
}
