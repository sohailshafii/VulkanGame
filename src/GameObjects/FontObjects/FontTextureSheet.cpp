
#include "FontTextureSheet.h"
#include "GameObjects/FontObjects/FreeTypeInterface.h"
#include <iostream>
#include <string>

FontTextureSheet::FontTextureSheet() {
	InitFreeTypeLibrary();
}

void FontTextureSheet::InitFreeTypeLibrary() {
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cerr << "FontTextureSheet: Could not init the FreeType Library.\n";
		return;
	}

#if __APPLE__
	const std::fontPath fontPathPrefix = "../../fonts/";
#else
	const std::string fontPathPrefix = "../fonts/";
#endif
	const std::string fontPath = fontPathPrefix + "oxanium/Oxanium-Medium.ttf";

	FT_Face face;
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
		std::cerr << "FontTextureSheet: Failed to load font.\n";
	}

	FT_Set_Pixel_Sizes(face, 0, 48);

	std::cout << "Successfully loaded font.\n";
}
