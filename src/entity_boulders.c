#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

#include "common.h"

#include "../res/sprite_boulders.h"

// Boulders Screen X value to add to current X per frame: Range 0 - 255 mapped to 32 - 143
static const uint8_t spr_sin_table_x[] = {
    // Scale factor of 16 x 80
    // 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, 0, -1, 0, -1, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, -1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, -1, 1, 0, -1, 0, 0, -1, -1, 0, -1, 0, -1, -1, -1, -1, -1, -1, -2, -1, -1, -1, -2, -1, -2, -1, -2, -1, -2, -2, -1, -2, -1, -2, -1, -2, -1, -1, -2, -1, -1, -2, -1, -1, 0, -1, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 2, 0, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 2, 3, 2, 2, 3, 2, 2, 1, 2, 1, 2, 2, 1, 1, 0, 1, 0, 1, -1, 0, 0, -1, -1, -2, 0, -3, -1, -2, -2, -2, -4, -2, -3, -4, -4, -3, -4, -4, -4, -5, -5, -5, -6, -6,
    // Scale factor of 20 x 80
    0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, 0, -1, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, -1, 0, 0, 0, -1, 0, 0, 0, 0, 1, -1, 0, 1, -1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -1, -1, -2, -1, -2, -1, -2, -2, -1, -2, -2, -2, -2, -1, -2, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2, -1, -2, -1, -2, -2, -1, -1, -2, -1, -1, -1, -1, -1, 0, -1, 0, -1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 2, 1, 2, 1, 2, 1, 2, 2, 2, 3, 2, 3, 3, 3, 3, 3, 3, 4, 3, 4, 4, 5, 5
};


// Boulders Size Sprite Select: Range 0 - 255 mapped to 32 - 143
// Should roughly match Screen Y LUT
static const uint8_t spr_select_by_count[] = {
   // Num Sprites = 9, Scaler = 16
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

// Boulders Screen Y location: Range 0 - 255 mapped to 32 - 143
static const uint8_t spr_y_by_count[] = {
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 38, 38, 38, 38, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 47, 47, 47, 47, 47, 48, 48, 48, 48, 49, 49, 50, 50, 50, 50, 50, 51, 51, 51, 51, 52, 52, 53, 54, 54, 54, 54, 54, 54, 55, 56, 56, 56, 56, 57, 57, 57, 57, 58, 59, 59, 60, 60, 60, 60, 60, 61, 61, 62, 62, 63, 63, 64, 64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 70, 70, 71, 71, 71, 71, 72, 74, 74, 74, 74, 75, 75, 77, 77, 78, 78, 78, 80, 80, 81, 81, 82, 83, 83, 85, 85, 86, 86, 87, 87, 88, 90, 90, 92, 92, 93, 93, 95, 95, 96, 98, 98, 99, 99, 102, 102, 104, 106, 106, 108, 108, 111, 111, 113, 113, 116, 119, 119, 124, 124, 130, 130, 142, 143,
};


// Boulders Screen X initial launch position: alternate a little so they don't show up all at once
static const uint8_t spr_launch_table_x[] = {17, 85, 35, 140, 52, 150, 70,  105};


#define SPRITE_COUNT_BOULDER 8u //20u // TODO: some kind of overflow and crash when >= 19u - something in move_metasprite
uint8_t entity_boulders_active;
uint8_t entity_boulder_x[SPRITE_COUNT_BOULDER];
uint8_t entity_boulder_count[SPRITE_COUNT_BOULDER];
uint8_t entity_boulder_mtspr;


void entity_boulders_init(void) {
    uint8_t c;
    for (c = 0u; c < SPRITE_COUNT_BOULDER; c++) {
        entity_boulder_x[c] = spr_launch_table_x[c];
        entity_boulder_count[c] = 0;
    }

    entity_boulders_active = 0;
}

void entity_boulders_add(void) {
}

uint8_t entity_boulders_update(uint8_t oam_high_water) {

    uint8_t idx;
    uint8_t spr_screen_y;
    uint8_t spr_count;

    // Slowly introduce more boulders
    if (entity_boulders_active < SPRITE_COUNT_BOULDER) {
        if (((uint8_t)sys_time & 0x1Fu) == 0x00)
            entity_boulders_active++;
    }

    // Update all sprites
    //
    // TODO: Once finalized consider unrolling this loop
    for (idx = 0u; idx < entity_boulders_active; idx++) {

        // Increment per-sprite counter and cache a temp copy
        spr_count = entity_boulder_count[idx]++;

        // Reset X on wraparound
        if (spr_count == 0)
            entity_boulder_x[idx] = spr_launch_table_x[idx];
        else
            entity_boulder_x[idx] += spr_sin_table_x[spr_count];

        // TODO: Fixme align sprite pixels to top of sprite instead of centered
        spr_screen_y = spr_y_by_count[spr_count] + 12;// 16;

        // TODO: Drop metasprites and just write directly to OAM
        //
        // Y position determines boulder size and Y location
        entity_boulder_mtspr = spr_select_by_count[spr_count];
        // TODO: hack-fix remove
if (entity_boulder_mtspr > 8) entity_boulder_mtspr = 8;

        // TODO: some kind of overflow and crash when SPRITE_COUNT_BOULDER >= 19u - something in move_metasprite
        // Setting this guard to < 37 results in a crash
        // Even < 36 I can start to see random GB Tile Data or map Data VRAM Overwrites / corruption
        // Is it a memory overwrite error, or an increasing race condition?
                // Might be in set_data()?? -> copy_vram (see BGB), or the interplay between these and a timing issue?
        // TODO: WORSE IN DMG MODE?? Problem here with vram when CPU usage gets high in DMG Mode
if (oam_high_water < 36) {
                oam_high_water += move_metasprite(sprite_boulders_metasprites[entity_boulder_mtspr],
                                                 (SPR_TILES_START_BOULDERS),
                                                 oam_high_water, entity_boulder_x[idx], spr_screen_y);
}
    }

    return (oam_high_water);
}