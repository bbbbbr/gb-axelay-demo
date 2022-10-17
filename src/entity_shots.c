#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

#include "common.h"
#include "input.h"
#include "lookup_tables.h"

#include "entity_boulders.h"
#include "entity_shots.h"

// For calculating tile in memory start addresses
#include "../res/sprite_boulders.h"
#include "../res/sprite_ship.h"
#include "../res/sprite_ship_canopy.h"
#include "../res/sprite_shots.h"


// uint8_t entity_shots_x[ENTITY_COUNT_SHOTS];
// uint8_t entity_shots_count[ENTITY_COUNT_SHOTS];
// bool entity_shots_active[ENTITY_COUNT_SHOTS];
// uint8_t entity_shots_mtspr;

uint8_t shots_active_total;
entity shots[ENTITY_COUNT_SHOTS];


uint8_t entity_shots_cooldown;


#define SHOTS_COUNT_SPEED    6u
#define SHOTS_X_OFFSET       8u   // Center shots on ship, which is 24 px wide (shot is 8 px wide)
#define SHOTS_COOLDOWN_READY 0u
#define SHOTS_COOLDOWN_START (60u / 5u) // 1/6 second


void entity_shots_init(void) {
    uint8_t idx;

    shots_active_total = 0;
    entity_shots_cooldown = SHOTS_COOLDOWN_READY;
    for (idx = 0; idx < ENTITY_COUNT_SHOTS; idx++)
        shots[idx].status = SPR_STATUS_HIDDEN;
}


// TODO: FIXME: THIS IS BUGGY WAY OF MANAGING ACTIVE SHOTS
// TODO: inline, optimize
void entity_shots_add(uint8_t x, uint8_t y) {

    uint8_t idx;

    if (entity_shots_cooldown)
        entity_shots_cooldown--;
    else {
        if (shots_active_total < ENTITY_COUNT_SHOTS) {

            // Find next free
            // TODO: (optimize this)
            idx = 0;
            while (shots[idx].status != SPR_STATUS_HIDDEN) {
                idx++;
            }

            shots[idx].x = x + SHOTS_X_OFFSET;
            shots[idx].y_counter = (y < 127) ? y * 2 : 255; // TODO: THIS IS A HACK, FIXME: Need a reverse mapping LUT for y -> 0 - 255 y LUT
            shots[idx].y_pos = LUT_y_pos_by_count[ shots[idx].y_counter ];
            shots[idx].status = SPR_STATUS_ACTIVE;
            shots_active_total++;
            entity_shots_cooldown = SHOTS_COOLDOWN_START;
        }
    }
}


void entity_shots_check_collisions(uint8_t spr_x_pos, uint8_t spr_y_pos, uint8_t shot_idx) {
/*
    uint8_t spr_y_bucket_top, spr_y_bucket_bottom;
    uint8_t col_group_idx;
    uint8_t boulder_id;

    // Collision test for boulders in the same Y partition
    spr_y_bucket_top = spr_y_pos >> PARTITION_BITSHIFT;
    spr_y_bucket_bottom = (spr_y_pos + 16) >> PARTITION_BITSHIFT;

    // check_collision
    for (col_group_idx = 0u; col_group_idx < boulders_col_group_y_count[spr_y_bucket_top]; col_group_idx++) {

        boulder_id = boulders_col_group_y[spr_y_bucket_top][col_group_idx];

        // TODO: actual hitbox
        // Collision test, X first since it's less likely to match than already partitioned Y
        if ((spr_x_pos + 8) >= boulders[boulder_id].x) {
            if (spr_x_pos <= (boulders[boulder_id].x + 16)) { // Shot is only 8 pixels wide?
                if (spr_y_pos <= (boulders[boulder_id].y_pos + 16)) {
                    if ((spr_y_pos + 16) >= boulders[boulder_id].y_pos) {

                        // TODO: explosions and stuff
                        boulders[boulder_id].status = SPR_STATUS_HIDDEN;
                        shots[shot_idx].status = SPR_STATUS_HIDDEN;
                        shots_active_total--; // TODO: FIXME: IS THIS FUXKED?
                    }
                }
            }
        }
    }

    if (spr_y_bucket_bottom != spr_y_bucket_top) {

        // check_collision
        for (col_group_idx = 0u; col_group_idx < boulders_col_group_y_count[spr_y_bucket_bottom]; col_group_idx++) {

            boulder_id = boulders_col_group_y[spr_y_bucket_bottom][col_group_idx];

            // TODO: actual hitbox
            // Collision test, X first since it's less likely to match than already partitioned Y
            if ((spr_x_pos + 8) >= boulders[boulder_id].x) {
                if (spr_x_pos <= (boulders[boulder_id].x + 16)) { // Shot is only 8 pixels wide?
                    if (spr_y_pos <= (boulders[boulder_id].y_pos + 16)) {
                        if ((spr_y_pos + 16) >= boulders[boulder_id].y_pos) {

                            // TODO: explosions and stuff
                            boulders[boulder_id].status = SPR_STATUS_HIDDEN;
                            shots[shot_idx].status = SPR_STATUS_HIDDEN;
                            shots_active_total--; // TODO: FIXME: IS THIS FUXKED?
                        }
                    }
                }
            }
        }
    }
*/
}

// WARNING: Max value in LUT_sprite_id_by_count[] should be identical
//          to total number of metasprites in *both* Shots and Boulders
//          Shots and boulders should also have identical metasprite pivot offsets
uint8_t entity_shots_update(uint8_t oam_high_water) {

    uint8_t idx;
    uint8_t spr_count;
    uint8_t spr_y_pos;

    if (shots_active_total) {

        // TODO: optimize
        for (idx = 0; idx < ENTITY_COUNT_SHOTS; idx++) {

            if (shots[idx].status != SPR_STATUS_HIDDEN) {

                // If still valid, move the shot and update it's sprite
                if (shots[idx].y_counter > SHOTS_COUNT_SPEED) {

                    spr_count = shots[idx].y_counter -= SHOTS_COUNT_SPEED;
                    spr_y_pos = shots[idx].y_pos = LUT_y_pos_by_count[spr_count];

                    entity_shots_check_collisions(shots[idx].x, spr_y_pos, idx);
                    // TODO: fixme
                    // if (oam_high_water < (40 - 1)) { // TODO: defines here

                        // Move ship and update metasprite
                        oam_high_water += move_metasprite(sprite_shots_metasprites[ LUT_sprite_id_by_count[spr_count] ],
                                                         (SPR_TILES_START_SHOTS),
                                                         oam_high_water,
                                                         shots[idx].x,
                                                         spr_y_pos);
                    // }
                } else {
                    // Remove shot
                    shots_active_total--;
                    shots[idx].status = SPR_STATUS_HIDDEN;
                }

            }
        }
    }

    return (oam_high_water);
}


/*
            // Check for collision with shots
            spr_y_pos = LUT_y_pos_by_count[spr_count];

            if (shots_active_total) {
                for (shot_idx = 0u; shot_idx < ENTITY_COUNT_SHOTS; shot_idx++) {
                    if (shots[shot_idx].status != SPR_STATUS_HIDDEN) {

                        // TODO: actual hitbox
                        // Collision test
                        if (spr_y_pos <= (shots[shot_idx].y_pos + 16)) {
                            if ((spr_y_pos + 16) >= shots[shot_idx].y_pos) {
                                if ((spr_x_pos + 16) >= shots[shot_idx].x) {
                                    if (spr_x_pos <= (shots[shot_idx].x + 8)) { // Shot is only 8 pixels wide?

                                        // TODO: explosions and stuff
//                                        boulders[idx].status = SPR_STATUS_HIDDEN;
                                        shots[shot_idx].status = SPR_STATUS_ACTIVE;
//                                        shots[shot_idx].status = SPR_STATUS_HIDDEN;
//                                        shots_active_total--; // TODO: FIXME: IS THIS FUXKED?
                                    }
                                }
                            }
                        }
                    }
                }
            } // End collision test
*/