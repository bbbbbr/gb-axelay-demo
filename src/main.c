#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include "input.h"

#include "../res/nes_map.h"
#include "../res/nes_tiles.h"

#include "../res/horizon_map.h"
#include "../res/horizon_tiles.h"

#include "map.h"


void init_gfx(void) {

    set_bkg_data(0, nes_tiles_count, nes_tiles);
    // set_bkg_tiles(0, 0, 32, 32, nes_map);
     set_bkg_tiles(0, (map_y >> 3) & 0x1Fu,                           // Start Y row: Map Y downshifted to tiles, clamped to HW map buffer dimensions (32 x 32)
                   nes_map_width, DEVICE_SCREEN_BUFFER_HEIGHT,        // Need full hardware map buffer Height due to stretching (TODO: change this to fir 18 high?)
                   &nes_map[((map_y >> 3) & 0x7Fu) * nes_map_width]); // Map Offset: Map Y downshifted to tiles, clamped to map Height (0x80 in tiles)



    if (_cpu == CGB_TYPE) {

        // Set CGB Palette
        set_bkg_palette(0, nes_num_pals, nes_pal_cgb);

        // Draw map tile colors/etc
        VBK_REG = 1; // Same as setting tiles above, but with tile attributes
        // set_bkg_tiles(0, 0, 32, 32, nes_map_attr);
         set_bkg_tiles(0, (map_y >> 3) & 0x1Fu,
                       nes_map_width, DEVICE_SCREEN_BUFFER_HEIGHT,
                       &nes_map_attr[((map_y >> 3) & 0x7Fu) * nes_map_width]);
        VBK_REG = 0; // Return to writing tile IDs
    } else {
        // Set DMG palette
        BGP_REG = DMG_PALETTE(DMG_BLACK, DMG_DARK_GRAY, DMG_LITE_GRAY, DMG_WHITE);
    }


    // set_bkg_tiles(0, 0, 20, 18, nes_map);

    // Set up horizon on alternate map (usually used for the Window)
    set_bkg_data(245u, horizon_tiles_count, horizon_tiles);
    set_win_submap(0, 0, 32, 4, horizon_map, horizon_map_width);

    SCX_REG = 0;
    SCY_REG = 0;

    SHOW_BKG;
    DISPLAY_ON;
}

// For now Scroll X amount needs to be +2 and init value of 1
// so that it avoids some PPU behavior that's different when SCX % 8 = 0
#define SCROLL_X_AMOUNT 2u
#define SCROLL_X_AMOUNT_TOP (SCROLL_X_AMOUNT / 2) // Scroll top region 1/2 as fast
#define SCROLL_Y_AMOUNT 1u

#define MAP_MIN_X 0
#define MAP_MAX_X ((((DEVICE_SCREEN_BUFFER_WIDTH) - (DEVICE_SCREEN_WIDTH)) * 8) - 1)


bool draw_queued_map = false;
uint16_t draw_queue_y_row = 0;

void main() {

    map_y = (nes_map_height - DEVICE_SCREEN_BUFFER_HEIGHT) * 8;  // Set to bottom of map
    map_x = 0;

    if (_cpu == CGB_TYPE) {
        // Use 2x CGB speed if available
        cpu_fast();
    }

    // TODO: fade-out
    init_gfx();

    map_isr_enable();

    // TODO: fade-in

    while (1) {
        wait_vbl_done();
        // SCX and SCY scroll regs get reset in the vblank ISR and updated in the HBlank ISR

        // == User Input ==

        if (KEY_PRESSED(J_LEFT)) {
            if (map_x > MAP_MIN_X) {
                map_x -= SCROLL_X_AMOUNT;
                map_x_top -= SCROLL_X_AMOUNT_TOP;
            }
        }
        else if (KEY_PRESSED(J_RIGHT)) {
            if (map_x < MAP_MAX_X) {
                map_x += SCROLL_X_AMOUNT;
                map_x_top += SCROLL_X_AMOUNT_TOP;
            }
        }


        // == Map Updates (from user input) ==

        // Auto-scrolling
        // if (sys_time & 0x01u)

        // map_y loops around, no need for min/max
        if (KEY_PRESSED(J_UP)) {
            map_y -= SCROLL_Y_AMOUNT;

            // Draw next *TOP* row if needed (TODO: reduce cpu spike, split to draw 1/8th for every 1 pixel scrolled)
            if ((map_y & 0x07) == 0) {
                draw_queued_map = true;
                draw_queue_y_row = (map_y >> 3); // Top of HW Map Buffer
            }
        }
        else if (KEY_PRESSED(J_DOWN)) {
            map_y += SCROLL_Y_AMOUNT;

            // Draw next *BOTTOM* row if needed
            if ((map_y & 0x07) == 0) {
                draw_queued_map = true;
                draw_queue_y_row = (map_y >> 3) + (DEVICE_SCREEN_BUFFER_HEIGHT - 1);  // Bottom of HW Map Buffer
            }
        }

        if (draw_queued_map) {
            draw_queued_map = false;

            // WARNING: Can't use set_bkg_data here since state of LCDC.3 (BG map selector) is unpredictable due to
            //          mid-frame flips, causing set_bkg_data to sometimes write to the map at 0x9C00 instead of 0x9800
            // Draw next row
            set_data( (uint8_t *)0x9800 + ((draw_queue_y_row & 0x1Fu) << 5), // Y Row clamped to HW map buffer dimensions (32 x 32) then x 32 ( << 5) to get row address in vram
                      &nes_map[(draw_queue_y_row & 0x7Fu) * nes_map_width],  // Map Offset: Map Y downshifted to tiles, clamped to map Height (0x80 in tiles)
                      nes_map_width);                                        // Write: 1 x row of tiles / bytes

            if (_cpu == CGB_TYPE) {
                // Draw map tile colors/etc
                VBK_REG = 1; // Same as setting tiles above, but with tile attributes
                // Draw next row
                set_data( (uint8_t *)0x9800 + ((draw_queue_y_row & 0x1Fu) << 5), // Y Row clamped to HW map buffer dimensions (32 x 32) then x 32 ( << 5) to get row address in vram
                          &nes_map_attr[(draw_queue_y_row & 0x7Fu) * nes_map_width],  // Map Offset: Map Y downshifted to tiles, clamped to map Height (0x80 in tiles)
                          nes_map_width);                                        // Write: 1 x row of tiles / bytes
                VBK_REG = 0; // Return to writing tile IDs
            }

        }

        UPDATE_KEYS();
    }
}
