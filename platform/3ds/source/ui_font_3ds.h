#pragma once

#include <stdint.h>

// 7 rows of 5 bits (bit 4 = leftmost), or NULL for characters without a glyph
// (space and anything unknown: the caller just advances).
const uint8_t *UiFont_Glyph(unsigned char c);

// An accented letter's accent: 2 rows of 5 bits, drawn on the rows kUiMarkRise and
// kUiMarkRise - 1 above the glyph's top (a free row between). NULL = none.
const uint8_t *UiFont_Mark(unsigned char c);

enum { kUiGlyphW = 5, kUiGlyphH = 7, kUiAdvance = 6, kUiMarkRise = 3 };

// The Chinese UI's face: 12x12 pixel cells drawn by Ark Pixel 12px (SIL OFL),
// generated into ui_cjk_font.inc by tools/gen_cjk_font.py. Only the characters
// the Chinese column of port/port_text_i18n.c uses are in the table.
// ui_cjk_font.inc holds the data; these are the accessors the renderers use.
#define UI_CJK_CELL_W 12
#define UI_CJK_CELL_H 12

// One glyph as UI_CJK_CELL_H rows of 2 bytes (bit 15 = leftmost pixel), or
// NULL when the character has none. `advance` receives the pen movement; an
// unknown character still advances a full cell so text never glues together.
const uint8_t *UiFont_CjkGlyph(uint32_t codePoint, int *advance);

// Pen movement for one code point, in pixels.
int UiFont_CjkAdvance(uint32_t codePoint);

// Width of UTF-8 text in the CJK face (no trailing space, like kUiGlyphH text).
int UiFont_CjkTextWidth(const char *utf8);
