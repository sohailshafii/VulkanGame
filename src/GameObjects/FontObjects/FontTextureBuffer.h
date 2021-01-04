#pragma once

#include <vulkan/vulkan.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "Resources/ResourceLoader.h"

class GfxDeviceManager;
class LogicalDeviceManager;

class FontTextureBuffer {
public:
	FontTextureBuffer(std::string const & fontFolderAndName);

	~FontTextureBuffer();

	unsigned char* GetBuffer() {
		return textureSheetBuffer;
	}

	unsigned int GetTextureWidth() const {
		return textureWidthPOT;
	}

	unsigned int GetTextureHeight() const {
		return textureHeightPOT;
	}

	unsigned int GetBytesPerPixel() const {
		return bytesPerPixel;
	}

	struct FontPositioningInfo {
		FontPositioningInfo() : rows(0), width(0),
			bitMapLeft(0), bitMapTop(0),
			advanceX(0), character(0) {
		}

		FontPositioningInfo(unsigned int rows,
			unsigned int width, int bitMapLeft,
			int bitMapTop, int advanceX,
			unsigned char character) :
			rows(rows), width(width),
			bitMapLeft(bitMapLeft), bitMapTop(bitMapTop),
			advanceX(advanceX), character(character) {
			textureCoordsBegin[0] = 0.0f;
			textureCoordsBegin[1] = 0.0f;
			textureCoordsEnd[0] = 0.0f;
			textureCoordsEnd[1] = 0.0f;
		}

		int rows;
		int width;
		float textureCoordsBegin[2];
		float textureCoordsEnd[2];
		int bitMapLeft;
		int bitMapTop;
		int advanceX;
		unsigned char character;
	};

	FontPositioningInfo GetPositioningInfo(unsigned char character) {
		return fontPositioningInfos[character];
	}

private:
	class FontRasterInfo {
	public:
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
			memcpy(this->buffer, srcBuffer, numBytes);
		}

		~FontRasterInfo() {
			if (buffer != nullptr) {
				delete[] buffer;
			}
		}

		int rows;
		int width;
		int pitch;
		unsigned char* buffer;
		int heightOffset,
			widthOffset;
		unsigned char character;
	};

	FT_Library InitFreeTypeLibrary();
	void BuildFonts(FT_Library freeTypeLibrary,
		std::vector<FontRasterInfo>& fontRasterInfos,
		std::string const & fontFolderAndName);
	bool ComputeFontTextureSize(std::vector<FontRasterInfo>& fontRasterInfos);

	void SetUpTextureCoords(std::vector<FontRasterInfo> const& rasterInfos);
	void BuildTextureSheet(std::vector<FontRasterInfo> const & rasterInfos);

	static constexpr int fontHeight = 20;
	static constexpr int horizSpaceBetweenFonts = 20;
	static constexpr int vertSpaceBetweenFonts = 20;
	static constexpr int numCharacterAcross = 25;
	static constexpr int maxTextureSize = 2048;

	unsigned char* textureSheetBuffer;
	unsigned int textureWidthPOT;
	unsigned int textureHeightPOT;
	unsigned int bytesPerPixel;
	std::map<unsigned char, FontPositioningInfo> fontPositioningInfos;
};
