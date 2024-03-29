//AUTOGENERATED FILE FROM png2asset
#ifndef METASPRITE_sprite_ship_H
#define METASPRITE_sprite_ship_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define sprite_ship_TILE_ORIGIN 0
#define sprite_ship_TILE_W 8
#define sprite_ship_TILE_H 16
#define sprite_ship_WIDTH 24
#define sprite_ship_HEIGHT 32
#define sprite_ship_TILE_COUNT 36
#define sprite_ship_PALETTE_COUNT 8
#define sprite_ship_COLORS_PER_PALETTE 4
#define sprite_ship_TOTAL_COLORS 32
#define sprite_ship_PIVOT_X 0
#define sprite_ship_PIVOT_Y 0
#define sprite_ship_PIVOT_W 16
#define sprite_ship_PIVOT_H 16

BANKREF_EXTERN(sprite_ship)

extern const palette_color_t sprite_ship_palettes[32];
extern const uint8_t sprite_ship_tiles[576];

extern const metasprite_t* const sprite_ship_metasprites[5];

#endif
