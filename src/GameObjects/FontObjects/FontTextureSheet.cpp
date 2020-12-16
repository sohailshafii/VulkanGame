
#include "FontTextureSheet.h"
#include "GameObjects/FontObjects/FreeTypeInterface.h"
#include <iostream>
#include <string>

FontTextureSheet::FontTextureSheet() {
	FT_Library freeTypeLibrary = InitFreeTypeLibrary();

	if (freeTypeLibrary != nullptr) {
		std::vector<FontRasterInfo> rasterInfos;
		BuildFonts(freeTypeLibrary, rasterInfos);

		int textureWidthPOT = 1, textureHeightPOT = 1;
		bool computedSizes =
			ComputeFontTextureSize(rasterInfos,
				textureWidthPOT, textureHeightPOT);

		if (computedSizes) {
			BuildTextureSheet(rasterInfos, textureWidthPOT,
				textureHeightPOT);
		}

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

void FontTextureSheet::BuildFonts(FT_Library freeTypeLibrary,
	std::vector<FontRasterInfo>& fontRasterInfos) {
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

	// have consistent height
	FT_Set_Pixel_Sizes(face, 0, fontHeight);
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

bool FontTextureSheet::ComputeFontTextureSize(std::vector<FontRasterInfo>& fontRasterInfos,
	int textureWidthPOT, int textureHeightPOT) {
	// with X number of pixels down, find the dimensions of the texture
	// in powers of two
	size_t numCharactersTotal = fontRasterInfos.size();
	size_t numRows = (size_t)ceil((float)numCharactersTotal / (float)numCharacterAcross);
	int fontHeightWithSpacing = fontHeight + horizSpaceBetweenFonts;
	int textureHeight = (int)numRows * fontHeightWithSpacing;
	textureHeightPOT = 1;
	while (textureHeightPOT < textureHeight &&
		textureHeightPOT < maxTextureSize) {
		textureHeightPOT *= 2;
	}
	if (textureHeightPOT > maxTextureSize) {
		std::cerr << "Could not make a power of two texture for height "
			<< textureHeight << " that is less than allowed size of "
			<< maxTextureSize << ".\n";
		return false;
	}

	int maxWidth = -1;
	int widthSoFar = 0;
	int currentHeight = 0;
	for (size_t i = 0; i < numCharactersTotal; i++) {
		auto & rasterInfo = fontRasterInfos[i];
		rasterInfo.heightOffset = currentHeight;
		rasterInfo.widthOffset = widthSoFar;
		// after a row completes, compare against max width
		// if row size is 25, row is indices 0-24. so on
		// row index 25, the first of the next row, we
		// see if the size of row 0-24 is the biggest so far
		if (i > 0 && i % numCharacterAcross == 0) {
			if (widthSoFar > maxWidth) {
				maxWidth = widthSoFar;
			}
			widthSoFar = 0;
			currentHeight += fontHeightWithSpacing;
		}

		widthSoFar += rasterInfo.width + vertSpaceBetweenFonts;
	}

	// measure width after last character
	if (widthSoFar > maxWidth) {
		maxWidth = widthSoFar;
	}
	textureWidthPOT = 1;
	while (textureWidthPOT < maxWidth &&
		textureWidthPOT < maxTextureSize) {
		textureWidthPOT *= 2;
	}
	if (textureWidthPOT > maxTextureSize) {
		std::cerr << "Could not make a power of two texture for width "
			<< maxWidth << " that is less than allowed size of "
			<< maxTextureSize << ".\n";
		return false;
	}

	std::cout << "Texture width and height: (" << textureWidthPOT <<
		" x " << textureHeightPOT << ").\n";
	return true;
}

void FontTextureSheet::BuildTextureSheet(std::vector<FontRasterInfo> const & rasterInfos,
	int textureWidthPOT, int textureHeightPOT) {
	unsigned char* textureSheetBuffer =
		new unsigned char[textureWidthPOT*textureHeightPOT];

	size_t numFonts = rasterInfos.size();
	for (size_t fontIndex = 0; fontIndex < numFonts; fontIndex++) {
		auto const& fontRasterInfo = rasterInfos[fontIndex];

		// go through all rows of font, copy to the correct spot in sheet
		int bufferRows = fontRasterInfo.rows;
		for (int rowIndex = 0; rowIndex < bufferRows; rowIndex++) {
			unsigned char* pointerToCurrRow = &textureSheetBuffer[
				(fontRasterInfo.heightOffset + rowIndex) * textureWidthPOT +
					fontRasterInfo.widthOffset];
			unsigned char* srcPointer = &fontRasterInfo.buffer[
				fontRasterInfo.width*rowIndex];
			// TODO: fix memory error and uncomment after the fact
			//memcpy(pointerToCurrRow, srcPointer, fontRasterInfo.width);
		}
	}

	delete[] textureSheetBuffer;
}
