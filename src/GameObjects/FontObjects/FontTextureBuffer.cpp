
#include "FontTextureBuffer.h"
#include "GameObjects/FontObjects/FreeTypeInterface.h"
#include "LogicalDeviceManager.h"
#include "GfxDeviceManager.h"
#include <iostream>
#include <string>

FontTextureBuffer::FontTextureBuffer() : textureSheetBuffer(nullptr) {
	FT_Library freeTypeLibrary = InitFreeTypeLibrary();

	if (freeTypeLibrary != nullptr) {
		std::vector<FontRasterInfo> rasterInfos;
		BuildFonts(freeTypeLibrary, rasterInfos);

		unsigned int textureWidthPOT = 1, textureHeightPOT = 1;
		bool computedSizes =
			ComputeFontTextureSize(rasterInfos);

		if (computedSizes) {
			BuildTextureSheet(rasterInfos);
		}

		FT_Done_FreeType(freeTypeLibrary);
	}
}

FontTextureBuffer::~FontTextureBuffer() {
	if (textureSheetBuffer != nullptr) {
		delete[] textureSheetBuffer;
	}
}

FT_Library FontTextureBuffer::InitFreeTypeLibrary() {
	FT_Library freeTypeLibrary;
	if (FT_Init_FreeType(&freeTypeLibrary)) {
		std::cerr << "FontTextureSheet: Could not init the FreeType Library.\n";
		return nullptr;
	}
	return freeTypeLibrary;
}

void FontTextureBuffer::BuildFonts(FT_Library freeTypeLibrary,
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

		if (bitmap.buffer == nullptr) {
			continue;
		}

		fontRasterInfos.push_back(FontRasterInfo(bitmap.rows,
			bitmap.width, bitmap.pitch, bitmap.buffer, c));

		fontPositioningInfos.push_back(FontPositioningInfo(bitmap.rows,
			bitmap.width, glyph->bitmap_left, glyph->bitmap_top, c));
	}

	FT_Done_Face(face);

	std::cout << "Successfully loaded font.\n";
}

bool FontTextureBuffer::ComputeFontTextureSize(std::vector<FontRasterInfo>& fontRasterInfos) {
	// with X number of pixels down, find the dimensions of the texture
	// in powers of two. first figure out how much minimum space is required
	size_t numCharactersTotal = fontRasterInfos.size();
	size_t numRows = (size_t)ceil((float)numCharactersTotal / (float)numCharacterAcross);
	unsigned int maxRowHeight = 0;
	unsigned int totalHeightSoFar = 0;
	for (size_t i = 0; i < numCharactersTotal; i++) {
		auto& rasterInfo = fontRasterInfos[i];
		rasterInfo.heightOffset = totalHeightSoFar;

		// new row? reset maxHeight found so far
		if (i > 0 && i % numCharacterAcross == 0) {
			totalHeightSoFar += maxRowHeight + horizSpaceBetweenFonts;
			maxRowHeight = 0;
		}
		if (rasterInfo.rows > maxRowHeight) {
			maxRowHeight = rasterInfo.rows;
		}
	}

	// take last row into account
	// the loop might end on the last character of the row
	totalHeightSoFar += maxRowHeight + horizSpaceBetweenFonts;

	textureHeightPOT = 1;
	while (textureHeightPOT < totalHeightSoFar &&
		textureHeightPOT < maxTextureSize) {
		textureHeightPOT *= 2;
	}
	if (textureHeightPOT > maxTextureSize) {
		std::cerr << "Could not make a power of two texture for height "
			<< totalHeightSoFar << " that is less than allowed size of "
			<< maxTextureSize << ".\n";
		return false;
	}

	unsigned int maxWidth = 0;
	unsigned int widthSoFar = 0;
	for (size_t i = 0; i < numCharactersTotal; i++) {
		auto & rasterInfo = fontRasterInfos[i];
		rasterInfo.widthOffset = widthSoFar;
		// after a row completes, compare against max width
		// if row size is 25, row is indices 0-24. so on
		// row index 25 (i % 25 = 0), the first of the next row, we
		// see if the size of row 0-24 is the biggest so far
		if (i > 0 && i % numCharacterAcross == 0) {
			if (widthSoFar > maxWidth) {
				maxWidth = widthSoFar;
			}
			widthSoFar = 0;
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

	bytesPerPixel = 1;
	std::cout << "Texture width and height: (" << textureWidthPOT <<
		" x " << textureHeightPOT << ").\n";
	return true;
}

void FontTextureBuffer::BuildTextureSheet(std::vector<FontRasterInfo> const & rasterInfos) {
	textureSheetBuffer =
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
			memcpy(pointerToCurrRow, srcPointer, fontRasterInfo.width);
		}
	}
}
