#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

#include "common.h"

#include "../res/sprite_boulders.h"

// const uint8_t spr_sin_table[] = {0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0};
const uint8_t spr_sin_table[] = {0, 1, 2, 3, -3, -2, -1, 0};

#define SPRITE_COUNT_BOULDER 10
uint8_t entity_boulder_x = 144u / 2u;
uint8_t entity_boulder_y = HORIZON_Y_START + 16u;
uint8_t entity_boulder_mtspr = 0;

void entity_boulders_init(void) {

}

uint8_t entity_boulders_update(uint8_t oam_high_water) {

    // Hide the metasprite or move it & apply any rotation settings
    // if (hide)
    //    hide_metasprite(sprite_boulders_metasprites[idx], SPR_NUM_START);

    entity_boulder_y++;
    if (entity_boulder_y > (144u + 16u)) {
        // Psuedo random launcing for now
        entity_boulder_x = (sys_time & 0x7Fu) + ((DEVICE_SCREEN_PX_WIDTH -  0x7Fu) / 2);
        entity_boulder_y = HORIZON_Y_START + 16u;
    }
    // entity_boulder_x += spr_sin_table[entity_boulder_y & 0x07u];

    // Y position determines boulder size and Y location - for now linear, later use a non-linear LUT that matches horizon
    entity_boulder_mtspr = (entity_boulder_y - (HORIZON_Y_START + 16u)) >> 4;
    oam_high_water = move_metasprite(sprite_boulders_metasprites[entity_boulder_mtspr],
                              (SPR_TILES_START_BOULDERS),
                              (SPR_NUM_START), entity_boulder_x, entity_boulder_y);

    return (oam_high_water);
}