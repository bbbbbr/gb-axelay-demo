#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

#include "common.h"
#include "input.h"

#include "../res/sprite_boulders.h"
#include "../res/sprite_ship.h"
#include "../res/sprite_ship_canopy.h"
#include "../res/sprite_shots.h"


// TODO: fixme: common.c or lookup_tables.c
extern const uint8_t spr_select_by_count[];
extern const uint8_t spr_y_by_count[];

#define SPRITE_COUNT_SHOTS_MAX  5u
uint8_t entity_shots_x[SPRITE_COUNT_SHOTS_MAX];
uint8_t entity_shots_count[SPRITE_COUNT_SHOTS_MAX];
bool entity_shots_active[SPRITE_COUNT_SHOTS_MAX];
uint8_t entity_shots_active_count;
uint8_t entity_shots_mtspr;


uint8_t entity_shots_cooldown;


#define SHOTS_COUNT_SPEED    6u
#define SHOTS_X_OFFSET       8u   // Center shots on ship, which is 24 px wide (shot is 8 px wide)
#define SHOTS_COOLDOWN_READY 0u
#define SHOTS_COOLDOWN_START (60u / 5u) // 1/6 second


void entity_shots_init(void) {
    uint8_t idx;

    entity_shots_active_count = 0;
    entity_shots_cooldown = SHOTS_COOLDOWN_READY;
    for (idx = 0; idx < SPRITE_COUNT_SHOTS_MAX; idx++)
        entity_shots_active[idx] = false;
}


// TODO: inline, optimize
void entity_shots_add(uint8_t x, uint8_t y) {

    uint8_t idx;

    if (entity_shots_cooldown)
        entity_shots_cooldown--;
    else {
        if (entity_shots_active_count < SPRITE_COUNT_SHOTS_MAX) {

            // Find next free
            // TODO: (optimize this)
            idx = 0;
            while (entity_shots_active[idx] == true) {
                idx++;
            }

            entity_shots_x[idx] = x + SHOTS_X_OFFSET;
            entity_shots_count[idx] = (y < 127) ? y * 2 : 255; // TODO: THIS IS A HACK, FIXME: Need a reverse mapping LUT for y -> 0 - 255 y LUT
            entity_shots_active[idx] = true;
            entity_shots_active_count++;
            entity_shots_cooldown = SHOTS_COOLDOWN_START;
        }
    }
}


// WARNING: Max value in spr_select_by_count[] should be identical
//          to total number of metasprites in *both* Shots and Boulders
//          Shots and boulders should also have identical metasprite pivot offsets
uint8_t entity_shots_update(uint8_t oam_high_water) {

    uint8_t idx;
    uint8_t spr_count;

    if (entity_shots_active_count) {

        // TODO: optimize
        for (idx = 0; idx < SPRITE_COUNT_SHOTS_MAX; idx++) {

            if (entity_shots_active[idx]) {

                // If still valid, move the shot and update it's sprite
                if (entity_shots_count[idx] > SHOTS_COUNT_SPEED) {

                    spr_count = entity_shots_count[idx] -= SHOTS_COUNT_SPEED;

                    // TODO: fixme
                    if (oam_high_water < (40 - 1)) { // TODO: defines here

                        // Move ship and update metasprite
                        oam_high_water += move_metasprite(sprite_shots_metasprites[ spr_select_by_count[spr_count] ],
                                                         (SPR_TILES_START_SHOTS),
                        // oam_high_water += move_metasprite(sprite_boulders_metasprites[ spr_select_by_count[spr_count] ],
                        //                                  (SPR_TILES_START_BOULDERS),
                                                         oam_high_water,
                                                         entity_shots_x[idx],
                                                         spr_y_by_count[spr_count]);
                    }
                } else {
                    // Remove shot
                    entity_shots_active_count--;
                    entity_shots_active[idx] = false;
                }

            }
        }
    }

    return (oam_high_water);
}