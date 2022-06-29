#ifndef MINES_CHARS
#define MINES_CHARS

#include <stdint.h>

static const uint8_t CLOCK_GLYPH[] = {
	0b0111110,
	0b1100011,
	0b1001101,
	0b1101011,
	0b0111110
};

static const uint8_t FLAG_GLYPH[] = {
	0b0000000,
	0b1111111,
	0b0011111,
	0b0001110,
	0b0000100
};

static const uint8_t MINE_GLYPH[] = {
	0b0101010,
	0b0011100,
	0b0111110,
	0b0011100,
	0b0101010
};

static const uint8_t UNREVEALED_GLYPH[] = {
	0b0000000,
	0b0000000,
	0b0001000,
	0b0000000,
	0b0000000
};

static const uint8_t SELECTED_GLYPH[] = {
	0b1111111,
	0b1111111,
	0b1111111,
	0b1111111,
	0b1111111
};

#endif
