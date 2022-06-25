#include <stdint.h>

const uint8_t BLANK_GLYPHS[] = {
	' ', '1', '2', '3', '4', '5', '6', '7', '8'
};

const uint8_t CLOCK_GLYPH[] = {
	0b00011100,
	0b00100010,
	0b01001111,
	0b00101010,
	0b00011100
};

const uint8_t FLAG_GLYPH[] = {
	0b00000000,
	0b11111111,
	0b00011111,
	0b00001110,
	0b00000100
};

const uint8_t MINE_GLYPH[] = {
	0b00101010,
	0b0011100,
	0b0111110,
	0b0011100,
	0b0101010
};

const uint8_t SPACE_GLYPH[] = {
	0b0000000,
	0b0011100,
	0b0011100,
	0b0011100,
	0b0000000
};

const uint8_t SELECTED_GLYPH[] = {
	0b0111110,
	0b0111110,
	0b0111110,
	0b0111110,
	0b0111110
};
