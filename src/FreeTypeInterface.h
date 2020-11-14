#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

class FreeTypeInterface {
public:
	FreeTypeInterface();
	~FreeTypeInterface();

private:
	FT_Library freeTypeLib;
};
