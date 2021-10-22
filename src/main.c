#include <gbdk/platform.h>
#include <stdint.h>
#include "input.h"

#include "../res/bigmap_map.h"
#include "../res/bigmap_tiles.h"

#include "map.h"

extern const uint8_t scy_horizon_offsets[];
extern const uint8_t * p_scy_horizon_offsets;

void map_isr_enable(void);


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

void main() {

    init_gfx();

    p_scy_horizon_offsets = scy_horizon_offsets;
    map_isr_enable();

    while (1) {
        wait_vbl_done();
        // Reset scy offset pointer at start of frame
        // TODO: move into Vblank ISR/etc
        SCY_REG = map_y;
        if (sys_time & 0x01u)
            map_y--;
        p_scy_horizon_offsets = scy_horizon_offsets;
    }
}
