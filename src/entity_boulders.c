#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

#include "common.h"
#include "lookup_tables.h"

#include "entity_boulders.h"
#include "entity_shots.h"

// For calculating tile in memory start addresses
#include "../res/sprite_boulders.h"

// Boulders Screen X value to add to current X per frame: Range 0 - 255 mapped to 32 - 143
static const uint8_t boulder_sin_table_x[] = {
    // Scale factor of 16 x 80
    // 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, 0, -1, 0, -1, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, -1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, -1, 1, 0, -1, 0, 0, -1, -1, 0, -1, 0, -1, -1, -1, -1, -1, -1, -2, -1, -1, -1, -2, -1, -2, -1, -2, -1, -2, -2, -1, -2, -1, -2, -1, -2, -1, -1, -2, -1, -1, -2, -1, -1, 0, -1, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 2, 0, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 2, 3, 2, 2, 3, 2, 2, 1, 2, 1, 2, 2, 1, 1, 0, 1, 0, 1, -1, 0, 0, -1, -1, -2, 0, -3, -1, -2, -2, -2, -4, -2, -3, -4, -4, -3, -4, -4, -4, -5, -5, -5, -6, -6,
    // Scale factor of 20 x 80
    0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, 0, -1, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, -1, 0, 0, 0, -1, 0, 0, 0, 0, 1, -1, 0, 1, -1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -1, -1, -2, -1, -2, -1, -2, -2, -1, -2, -2, -2, -2, -1, -2, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2, -1, -2, -1, -2, -2, -1, -1, -2, -1, -1, -1, -1, -1, 0, -1, 0, -1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 2, 1, 2, 1, 2, 1, 2, 2, 2, 3, 2, 3, 3, 3, 3, 3, 3, 4, 3, 4, 4, 5, 5
};


// Boulders Screen X initial launch position: alternate a little so they don't show up all at once
static const uint8_t boulder_launch_table_x[] = {17, 85, 35, 140, 52, 150, 70,  105};


uint8_t boulders_active_total;
entity boulders[ENTITY_COUNT_BOULDERS];



// uint8_t boulders_col_group_y_count[PARTITION_COUNT];
//uint8_t boulders_col_group_y[PARTITION_COUNT][ENTITY_COUNT_BOULDERS]; // TODO: per-partition bucket size can probably be reduced by 1/2 at least
// uint8_t boulders_col_group_x[PARTITION_COUNT]; // TODO: per-partition bucket size can probably be reduced by 1/2 at least

void entity_boulders_init(void) {
    uint8_t c;
    for (c = 0u; c < ENTITY_COUNT_BOULDERS; c++) {
        boulders[c].x = boulder_launch_table_x[c];
        boulders[c].y_counter = 0;
        // boulders[c].status = SPR_STATUS_HIDDEN;
        boulders_active[c] = SPR_STATUS_HIDDEN;
    }

    boulders_active_total = 0;
}



void entity_boulders_update_col_groups(void) __naked {
/*
    uint8_t idx;
    uint8_t * p_col_group = (uint8_t *)BOULDER_COL_GROUP_X_ADDR;
    uint8_t * p_shadow_oam = (uint8_t *)shadow_OAM; // OAM: [y, x, tile_id, flags]

    // Reset collision table
    for (idx = 0u; idx < boulders_active_total; idx++)
        *p_col_group = 0;

    // // Update collision table
    // for (idx = 0u; idx < (1 << boulders_active_total); idx<<1) {
    //     *(p_col_group + (*p_shadow_oam >>  PARTITION_BITSHIFT)) = idx;
    //     p_shadow_oam++;// += 4;
    // }
*/


// BROKEN? THIS WAS ENABLED
/*
    __asm \

        // Zero out collision group array

        xor   a
        ld    hl, #_boulders_col_group_x
        .rept 8
            ld  (hl+), a
        .endm

// TODO:
//   ONLY CALC FOR ACTIVE BOULDERS
//   properly handle 2x sprites for every


        // * boulders_col_group_x[]
        // * boulders_active[]
        // * shadow_OAM[]
        //
        // Counter 0 .. 7
        // Bitwise counter 1 .. 128

//        n_bit = 1;
//        while (counter != 8 or 128) {
//            If (p_boulders_active[n] != 0) {
//                temp_bucket = _shadow_OAM[(n * 8) + 1] >> 5;
//                boulders_col_group_x[ temp_bucket ] |= n_bit;
//            }
//            n_bit <<= 1;
//            n++;
//        }
//

        ld    h, #_boulders_col_group_x >> 8   // High byte for into collision array
        ld    l, (#_boulders_active)       // Counter
        ld    e, #0x01                     // Bitwise counter <-- needs to match boulders active
        ld    bc, #(_shadow_OAM + 1)       // Start at last X. OAM: [n+0]=y, [n+1]=x, [n+2]=tile_id, [n+3]=flags]

    boulder_entity_col_group_loop$:

        ld    l, d         // D has _boulders_active lower address byte
        ld    a, (hl)      // test if _boulders_active[n] == inactive
        jr    z, boulder_entity_col_group_loop_skip_inactive$

        ld    a, (bc)   // Calculate X collision group for boulder (reduce C to 3 bits)
        swap  a
        rra
        and   a, #0x07

        ld    h, #_boulders_col_group_x >> 8   // Load upper address bit of boulders active array
// L COUNTER GETS WIPED OUT HERE        <<<<<<-----
        ld    l, a      // Use collision group as lower address byte to index collision array
        ld    a, e
        or    a, (hl)   // OR in the Load bitwise counter and save it
        ld    (hl), a


    boulder_entity_col_group_loop_skip_inactive$:
        ld    a, #0x08  // Check counter for loop exit
        add   a, c      // Move to next object X value in OAM
        ld    c, a

        rl    e         // Upshift bitwise counter

        ld    a, #(0x08 + 0x08)  // Check counter for loop exit
        sub   a, d
        jr    nz, boulder_entity_col_group_loop$
    boulder_entity_col_group_loop_done$:

        // .rept 4
        //     inc   c
        // .endm

    __endasm;
*/
/*
//     //            boulders_col_group_x[spr_x_pos >> PARTITION_BITSHIFT] |= 1 << idx;
//             //BOULDER_COL_GROUP_X_ADDR_MSB
//             // uint8_t * p_addr =  (uint8_t *)(((uint16_t )(&boulders_col_group_x) << 8) || (spr_x_pos >> PARTITION_BITSHIFT));
//             *(uint8_t *)(BOULDER_COL_GROUP_X_ADDR | (spr_x_pos >> PARTITION_BITSHIFT)) |= 1 << idx;
//             // boulders_col_group_x[spr_x_pos >> PARTITION_BITSHIFT] |= 1 << idx;
//     __endasm;
*/
}

void entity_boulders_add(void) {
}


uint8_t spr_y_pos;
uint8_t spr_x_pos;
uint8_t spr_y_bucket;

// TODO: optimize: make oam_high_water global and don't pass it between functions
uint8_t entity_boulders_update(uint8_t oam_high_water) {

    uint8_t idx;
    uint8_t spr_count;
    // uint8_t spr_y_pos;
    // uint8_t spr_x_pos;
    // uint8_t spr_y_bucket;

    // // Reset partition group counters
    // for (idx = 0u; idx < PARTITION_COUNT; idx++) {
    //     boulders_col_group_x[idx] = 0;
    //     // boulders_col_group_y_count[idx] = 0;
    // }

    // TODO: change to spawning on a timer, like shots do
    // Slowly introduce more boulders
    if (boulders_active_total < ENTITY_COUNT_BOULDERS) {
        if (((uint8_t)sys_time & 0x1Fu) == 0x00) {
            boulders_active[boulders_active_total] = SPR_STATUS_ACTIVE;
            //boulders[boulders_active_total].status = SPR_STATUS_ACTIVE;
            boulders_active_total++;
        }
    }

    // Update all sprites
    //
    // TODO: Once finalized consider unrolling this loop
    // for (idx = 0u; idx < boulders_active_total; idx++) {
    for (idx = 0u; idx < boulders_active_total; idx++) {

        // Increment per-sprite counter and cache a temp copy
        spr_count = boulders[idx].y_counter++;

        // Reset X on wraparound
        if (spr_count == 0) {
            // boulders[idx].status = SPR_STATUS_ACTIVE;
            boulders_active[idx] = SPR_STATUS_ACTIVE;
            spr_x_pos = boulders[idx].x = boulder_launch_table_x[idx];
        }
        else
            spr_x_pos = boulders[idx].x += boulder_sin_table_x[spr_count];

        // Get warped Y position from linear Y counter lookup
        spr_y_pos = LUT_y_pos_by_count[spr_count];


        // TODO: Drop metasprites and just write directly to OAM
        // if (oam_high_water < (40u - 2u)) {
        //if (boulders[idx].status != SPR_STATUS_HIDDEN) {
        if (boulders_active[idx] != SPR_STATUS_HIDDEN) {
            oam_high_water += move_metasprite(sprite_boulders_metasprites[ LUT_sprite_id_by_count[spr_count] ],
                                             (SPR_TILES_START_BOULDERS),
                                             oam_high_water,
                                             spr_x_pos,
                                             spr_y_pos);

            // TODO: Use LUT for collision size
            // Then store Y top and bottom position in relevant collision group
            // Top


            // Generates massively inefficient code
/*          spr_y_bucket = spr_y_pos >> PARTITION_BITSHIFT;
            boulders_col_group_y[spr_y_bucket][boulders_col_group_y_count[spr_y_bucket]] = idx;
            boulders_col_group_y_count[spr_y_bucket]++;
            // Bottom if in a different partition
            if (((spr_y_pos + 16) >> PARTITION_BITSHIFT) != (spr_y_bucket)) {
                spr_y_bucket++; // If bottom partition is different, it's just the next one
                boulders_col_group_y[(spr_y_pos + 16) >> PARTITION_BITSHIFT][boulders_col_group_y_count[spr_y_bucket]] = idx;
                boulders_col_group_y_count[(spr_y_pos + 16) >> PARTITION_BITSHIFT]++;
            }
*/
        }
        // }
    }

    entity_boulders_update_col_groups();
    return (oam_high_water);
}
