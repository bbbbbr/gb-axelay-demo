#include <gbdk/platform.h>
#include <stdint.h>
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

        // Use 2x CGB speed if we have it
        cpu_fast();

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

void main() {

    map_y = 0; // (nes_map_height - 32u) * 8;
    map_x = 0;

    // TODO: fade-out
    init_gfx();


    map_isr_enable();

    // TODO: fade-in    

    while (1) {
        wait_vbl_done();
        // These get reset in the vblank ISR and updated in the HBlank ISR
        // SCX_REG = map_x;
        // SCY_REG = map_y;

#define MAP_MIN_X 0
#define MAP_MAX_X ((((DEVICE_SCREEN_BUFFER_WIDTH) - (DEVICE_SCREEN_WIDTH)) * 8) - 1)

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

        if (KEY_PRESSED(J_UP))
            map_y -= SCROLL_Y_AMOUNT;
        else if (KEY_PRESSED(J_DOWN))
            map_y += SCROLL_Y_AMOUNT;

        // if (sys_time & 0x01u)
        //     map_y--;

        UPDATE_KEYS();
    }
}
