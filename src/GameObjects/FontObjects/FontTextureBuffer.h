#pragma once

#include <vulkan/vulkan.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <memory>
#include <string>
#include "Resources/ResourceLoader.h"

class GfxDeviceManager;
class LogicalDeviceManager;

class FontTextureBuffer {
public:
	FontTextureBuffer();

	~FontTextureBuffer();

private:
	struct FontRasterInfo {
		FontRasterInfo() : rows(0), width(0),
			pitch(0), buffer(nullptr), heightOffset(0),
			widthOffset(0), character(0) {
		}

		FontRasterInfo(FontRasterInfo const &other) {
			this->rows = other.rows;
			this->width = other.width;
			this->pitch = other.pitch;
			this->heightOffset = other.heightOffset;
			this->widthOffset = other.widthOffset;
			int numBytes = rows * width;
			this->buffer = new unsigned char[numBytes];
			this->character = other.character;
			memcpy(this->buffer, other.buffer, numBytes);
		}

		FontRasterInfo(FontRasterInfo&& other)
		{
			this->rows = other.rows;
			this->width = other.width;
			this->pitch = other.pitch;
			this->buffer = other.buffer;
			this->heightOffset = other.heightOffset;
			this->widthOffset = other.widthOffset;
			this->character = other.character;
			other.buffer = nullptr;
		}

		FontRasterInfo(unsigned int rows, unsigned int width,
			int pitch, unsigned char* srcBuffer, unsigned char character) {
			this->rows = rows;
			this->width = width;
			this->pitch = pitch;
			this->heightOffset = 0;
			this->widthOffset = 0;
			int numBytes = rows * width;
			this->buffer = new unsigned char[numBytes];
			this->character = character;
			memcpy(this->buffer, buffer, numBytes);
		}

		~FontRasterInfo() {
			if (buffer != nullptr) {
				delete[] buffer;
			}
		}

		unsigned int rows;
		unsigned int width;
		int pitch;
		unsigned char* buffer;
		unsigned int heightOffset,
			widthOffset;
		unsigned char character;
	};

	struct FontPositioningInfo {
		FontPositioningInfo(unsigned int rows,
			unsigned int width, signed int bitMapLeft,
			signed int bitMapRight, unsigned char character) :
			rows(rows), width(width),
			bitMapLeft(bitMapLeft), bitMapRight(bitMapRight),
			character(character) {
		}

		unsigned int rows;
		unsigned int width;
		signed int bitMapLeft;
		signed int bitMapRight;
		unsigned char character;
	};

	FT_Library InitFreeTypeLibrary();
	void BuildFonts(FT_Library freeTypeLibrary,
		std::vector<FontRasterInfo>& fontRasterInfos);
	bool ComputeFontTextureSize(std::vector<FontRasterInfo>& fontRasterInfos);

	void BuildTextureSheet(std::vector<FontRasterInfo> const & rasterInfos);

	static constexpr int fontHeight = 20;
	static constexpr int horizSpaceBetweenFonts = 5;
	static constexpr int vertSpaceBetweenFonts = 5;
	static constexpr int numCharacterAcross = 25;
	static constexpr int maxTextureSize = 2048;

	unsigned char* textureSheetBuffer;
	unsigned int textureWidthPOT;
	unsigned int textureHeightPOT;
	unsigned int bytesPerPixel;
	std::vector<FontPositioningInfo> fontPositioningInfos;
};
