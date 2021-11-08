#include <gbdk/platform.h>
#include <stdint.h>
#include "input.h"

#include "../res/bigmap_map.h"
#include "../res/bigmap_tiles.h"

#include "map.h"


void init_gfx(void) {

    set_bkg_data(0, 241u, bigmap_tiles);

    set_bkg_submap(0, 0, 32, 32, bigmap_map, bigmap_mapWidth);
    // set_bkg_tiles(0, 0, 20, 18, bigmap_map);

    SCX_REG = 0;
    SCY_REG = 0;

    SHOW_BKG;
    DISPLAY_ON;
}


uint8_t map_y = 0;
uint8_t map_x = 0;
uint8_t map_x_top = 0;

// For now Scroll X amount needs to be +2 and init value of 1
// so that it avoids some PPU behavior that's different when SCX % 8 = 0
#define SCROLL_X_AMOUNT 2u
#define SCROLL_X_AMOUNT_TOP (SCROLL_X_AMOUNT / 2) // Scroll top region 1/2 as fast
#define SCROLL_Y_AMOUNT 1u

void main() {

    init_gfx();

    map_isr_enable();

    while (1) {
        wait_vbl_done();
        // These get reset in the vblank ISR and updated in the HBlank ISR
        // SCX_REG = map_x;
        // SCY_REG = map_y;

        if (KEY_PRESSED(J_LEFT)) {
            map_x -= SCROLL_X_AMOUNT;
            map_x_top -= SCROLL_X_AMOUNT_TOP;
        }
        else if (KEY_PRESSED(J_RIGHT)) {
            map_x += SCROLL_X_AMOUNT;
            map_x_top += SCROLL_X_AMOUNT_TOP;
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
