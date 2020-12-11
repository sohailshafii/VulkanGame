
#include "FontTextureSheet.h"
#include "GameObjects/FontObjects/FreeTypeInterface.h"
#include <iostream>
#include <string>

FontTextureSheet::FontTextureSheet() {
	FT_Library freeTypeLibrary = InitFreeTypeLibrary();

	if (freeTypeLibrary != nullptr) {
		BuildFonts(freeTypeLibrary);

		FT_Done_FreeType(freeTypeLibrary);
	}
}

FT_Library FontTextureSheet::InitFreeTypeLibrary() {
	FT_Library freeTypeLibrary;
	if (FT_Init_FreeType(&freeTypeLibrary)) {
		std::cerr << "FontTextureSheet: Could not init the FreeType Library.\n";
		return nullptr;
	}
	return freeTypeLibrary;
}

void FontTextureSheet::BuildFonts(FT_Library freeTypeLibrary) {
#if __APPLE__
	const std::fontPath fontPathPrefix = "../../fonts/";
#else
	const std::string fontPathPrefix = "../fonts/";
#endif
	const std::string fontPath = fontPathPrefix + "oxanium/Oxanium-Medium.ttf";

	FT_Face face;
	if (FT_New_Face(freeTypeLibrary, fontPath.c_str(), 0, &face)) {
		std::cerr << "FontTextureSheet: Failed to load font.\n";
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);
	std::vector<FontRasterInfo> fontRasterInfos;
	for (unsigned char c = 0; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cerr << "FontTextureSheet: Failed to load glyph!\n";
			continue;
		}

		auto glyph = face->glyph;
		auto& bitmap = glyph->bitmap;
		fontRasterInfos.push_back(FontRasterInfo(bitmap.width,
			bitmap.rows, bitmap.pitch, bitmap.buffer));

		fontPositioningInfos.push_back(FontPositioningInfo(bitmap.width,
			bitmap.rows, glyph->bitmap_left, glyph->bitmap_top));
	}

	FT_Done_Face(face);

	std::cout << "Successfully loaded font.\n";
}
