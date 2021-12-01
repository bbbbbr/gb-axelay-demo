#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <gb/isr.h>

// #include "input.h"
#include "common.h"
#include "input.h"

#include "map_scroll.h"

#include "../res/nes_map.h"

// For now Scroll X amount needs to be +2 and init value of 1
// so that it avoids some PPU behavior that's different when SCX % 8 = 0
#define SCROLL_X_AMOUNT 2u
#define SCROLL_X_AMOUNT_TOP (SCROLL_X_AMOUNT / 2) // Scroll top region 1/2 as fast
#define SCROLL_Y_AMOUNT 1u
#define MAP_MIN_X 0
#define MAP_MAX_X ((((DEVICE_SCREEN_BUFFER_WIDTH) - (DEVICE_SCREEN_WIDTH)) * 8) - 1)

// Globals
uint16_t map_y = 0;
uint8_t  map_x = 0;
uint8_t  map_x_top = 0;

// Static Locals
uint8_t  draw_queued_map = false;
static uint16_t draw_queue_y_row = 0;

uint8_t * p_vram_dest;
uint8_t * p_map_src;
uint8_t * p_map_attr_src;


// Reset vars and draw initial map
void map_scroll_init(void) {
    map_y = (nes_map_height - DEVICE_SCREEN_BUFFER_HEIGHT) * 8;  // Set to bottom of map
    map_x = 0;

    // Reset scrolling
    SCX_REG = 0;
    SCY_REG = 0;

    map_scroll_redraw_all();
}


// Redraw entire map
void map_scroll_redraw_all(void) {

    set_bkg_tiles(0, (map_y >> 3) & 0x1Fu,                           // Start Y row: Map Y downshifted to tiles, clamped to HW map buffer dimensions (32 x 32)
                  nes_map_width, DEVICE_SCREEN_BUFFER_HEIGHT,        // Need full hardware map buffer Height due to stretching (TODO: change this to fir 18 high?)
                  &nes_map[((map_y >> 3) & 0x7Fu) * nes_map_width]); // Map Offset: Map Y downshifted to tiles, clamped to map Height (0x80 in tiles)

    if (_cpu == CGB_TYPE) {

        // Draw map tile colors/etc
        VBK_REG = 1; // Same as setting tiles above, but with tile attributes
        // set_bkg_tiles(0, 0, 32, 32, nes_map_attr);
         set_bkg_tiles(0, (map_y >> 3) & 0x1Fu,
                       nes_map_width, DEVICE_SCREEN_BUFFER_HEIGHT,
                       &nes_map_attr[((map_y >> 3) & 0x7Fu) * nes_map_width]);
        VBK_REG = 0; // Return to writing tile IDs
    }
}


// Scroll based on user input
void map_scroll_update(void) {

    // Left / Right movement (Locked to Hardware Map Buffer Size, No Wrapping)
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

    // Map Up/Down scrolling + prep for tile loading

    // Auto-scrolling Vertical Scrolling
    if (sys_time & 0x01u) {
        map_y -= SCROLL_Y_AMOUNT;

        // Draw next *TOP* row if needed
        if ((map_y & 0x07) == 0) {

            draw_queued_map = MAP_SCROLL_CHUNK_COUNT;
            draw_queue_y_row = (map_y >> 3); // Top of HW Map Buffer

            p_vram_dest = (uint8_t *)0x9800 + ((draw_queue_y_row & 0x1Fu) << 5);
            uint16_t map_offset = (draw_queue_y_row & 0x7Fu) * nes_map_width;
            p_map_src = &nes_map[map_offset];
            p_map_attr_src = &nes_map_attr[map_offset];
        }
        // Scroll updates now get applied in the trailing vblank ISR
    }

    // == Turned Off:  Manual Vertical Scrolling ==
    //
    // NOTE: No longer compatible with current drawing, would need to revert to non-chunk code below
    //       and updated p_vram_dest, p_map_src, p_map_attr_src
    //
    // map_y loops around, no need for min/max
    // if (KEY_PRESSED(J_UP)) {
    //     map_y -= SCROLL_Y_AMOUNT;
    //     // Draw next *TOP* row if needed (TODO: reduce cpu spike, split to draw 1/8th for every 1 pixel scrolled)
    //     if ((map_y & 0x07) == 0) {
    //         draw_queued_map = true;
    //         draw_queue_y_row = (map_y >> 3); // Top of HW Map Buffer
    //     }
    // }
    // else if (KEY_PRESSED(J_DOWN)) {
    //     map_y += SCROLL_Y_AMOUNT;
    //     // Draw next *BOTTOM* row if needed
    //     if ((map_y & 0x07) == 0) {
    //         draw_queued_map = true;
    //         draw_queue_y_row = (map_y >> 3) + (DEVICE_SCREEN_BUFFER_HEIGHT - 1);  // Bottom of HW Map Buffer
    //     }
    // }


    // == Map Scroll Drawing ==
    //
    // NOTE: Map scroll tile updating is now moved to the end of the VBlank ISR (~line 145)
    //       It helps to avoid the VRAM write exceptions that were happening

/*    if (draw_queued_map) {
        draw_queued_map = false;

        // WARNING: Can't use set_bkg_data here since state of LCDC.3 (BG map selector) is unpredictable due to
        //          mid-frame flips, causing set_bkg_data to sometimes write to the map at 0x9C00 instead of 0x9800
        // Draw next row
        set_data( p_vram_dest, // Y Row clamped to HW map buffer dimensions (32 x 32) then x 32 ( << 5) to get row address in vram
                  p_map_src,  // Map Offset: Map Y downshifted to tiles, clamped to map Height (0x80 in tiles)
                  nes_map_width);                                        // Write: 1 x row of tiles / bytes

        if (_cpu == CGB_TYPE) {
            // Draw map tile colors/etc
            VBK_REG = 1; // Same as setting tiles above, but with tile attributes
            // Draw next row
            set_data( p_vram_dest, // Y Row clamped to HW map buffer dimensions (32 x 32) then x 32 ( << 5) to get row address in vram
                      p_map_attr_src,  // Map Offset: Map Y downshifted to tiles, clamped to map Height (0x80 in tiles)
                      nes_map_width);                                        // Write: 1 x row of tiles / bytes
            VBK_REG = 0; // Return to writing tile IDs
        }

    }
*/
}

