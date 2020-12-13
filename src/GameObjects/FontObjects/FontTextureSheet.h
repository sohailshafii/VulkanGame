#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>

class FontTextureSheet {
public:
	FontTextureSheet();
private:
	struct FontRasterInfo {
		FontRasterInfo() : rows(0), width(0),
			pitch(0), buffer(nullptr) {
		}

		FontRasterInfo(FontRasterInfo const &other) {
			this->rows = other.rows;
			this->width = other.width;
			this->pitch = other.pitch;
			int numBytes = rows * width;
			this->buffer = new unsigned char[numBytes];
			memcpy(this->buffer, other.buffer, numBytes);
		}

		FontRasterInfo(FontRasterInfo&& other)
		{
			this->rows = other.rows;
			this->width = other.width;
			this->pitch = other.pitch;
			this->buffer = other.buffer;
			other.buffer = nullptr;
		}

		FontRasterInfo(unsigned int rows, unsigned int width,
			int pitch, unsigned char* srcBuffer) {
			this->rows = rows;
			this->width = width;
			this->pitch = pitch;
			int numBytes = rows * width;
			this->buffer = new unsigned char[numBytes];
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
	};

	struct FontPositioningInfo {
		FontPositioningInfo(unsigned int rows,
			unsigned int width, signed int bitMapLeft,
			signed int bitMapRight) : rows(rows), width(width),
			bitMapLeft(bitMapLeft), bitMapRight(bitMapRight) {
		}

		unsigned int rows;
		unsigned int width;
		signed int bitMapLeft;
		signed int bitMapRight;
	};

	FT_Library InitFreeTypeLibrary();
	void BuildFonts(FT_Library freeTypeLibrary,
		std::vector<FontRasterInfo>& fontRasterInfos);
	bool ComputeFontTextureSize(std::vector<FontRasterInfo>& fontRasterInfos,
		int textureWidthPOT, int textureHeightPOT);

	std::vector<FontPositioningInfo> fontPositioningInfos;
	static constexpr int fontHeight = 20;
	static constexpr int numCharacterAcross = 25;
	static constexpr int maxTextureSize = 2048;
};
