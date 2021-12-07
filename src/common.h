
#ifndef common_H
#define common_H

#define HORIZON_Y_START (32 - 1) // One line before desired start line

#define SPR_NUM_START            0
#define SPR_TILES_START_BOULDERS (SPR_NUM_START)
#define SPR_TILES_START_SHIP     (SPR_TILES_START_BOULDERS + sprite_boulders_TILE_COUNT)
#define SPR_TILES_START_SHIP_CANOPY (SPR_TILES_START_SHIP + sprite_ship_TILE_COUNT)
#define SPR_TILES_START_SHOTS       (SPR_TILES_START_SHIP_CANOPY + sprite_ship_canopy_TILE_COUNT)

#endif