
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

#include "../res/nes_map.h"
#include "../res/nes_tiles.h"
#include "../res/horizon_map.h"
#include "../res/horizon_tiles.h"
#include "../res/sprite_boulders.h"

#include "input.h"
#include "common.h"

#include "map_fx.h"
#include "map_scroll.h"
#include "player_input.h"
#include "entity_boulders.h"

void init_gfx_map() {

    set_bkg_data(0, nes_tiles_count, nes_tiles);

    if (_cpu == CGB_TYPE) {
        // Set CGB Palette
        set_bkg_palette(0, nes_num_pals, nes_pal_cgb);
    } else {
        // Set DMG palette
        BGP_REG = DMG_PALETTE(DMG_BLACK, DMG_DARK_GRAY, DMG_LITE_GRAY, DMG_WHITE);
    }

    // Set up horizon on alternate map (usually used for the Window)
    set_bkg_data(245u, horizon_tiles_count, horizon_tiles);
    set_win_submap(0, 0, 32, 4, horizon_map, horizon_map_width);
}


void init_gfx_sprites() {

//    uint8_t sprite_idx = 0;
//    sprite_idx += sprite_boulders.num_tiles;

    // Load metasprite tile data into VRAM
    // set_sprite_data((SPR_TILES_START_BOULDERS), sprite_boulders.num_tiles, sprite_boulders.data);
    // Bug in png2gbtiles, incorrect tile count for 16x16 sprite, fix by *2
    set_sprite_data((SPR_TILES_START_BOULDERS), sprite_boulders_TILE_COUNT * 2, sprite_boulders_tiles);

    SPRITES_8x16;

    if (_cpu == CGB_TYPE) {
        // Set CGB Palette
        set_sprite_palette(0, nes_num_pals, nes_pal_cgb);
    } else {
        // Set DMG palette
        OBP0_REG = DMG_PALETTE(DMG_BLACK, DMG_BLACK, DMG_WHITE, DMG_WHITE);
    }
}


void init_gfx(void) {

    init_gfx_map();
    init_gfx_sprites();

    SHOW_BKG;
    SHOW_SPRITES;

    DISPLAY_ON;
}

void init(void) {


    if (_cpu == CGB_TYPE) {
        // Use 2x CGB speed if available
        cpu_fast();
    }

    // TODO: bind this to a button press
    initrand(sys_time);

    // TODO: fade-out
    init_gfx();

    map_scroll_init();
    entity_boulders_init();

    map_fx_isr_enable();

    // TODO: fade-in
}


uint8_t oam_high_water = 0;


void main() {

    init();

    while (1) {
        wait_vbl_done();
        // SCX and SCY scroll regs get reset in the vblank ISR and updated in the HBlank ISR

        // == User Input ==
        map_scroll_update();

        // == Sprites ==
        oam_high_water = 0;
        oam_high_water = entity_boulders_update(oam_high_water);

        // Hide rest of the hardware sprites, because amount of sprites differ between animation frames.
        hide_sprites_range(oam_high_water, 40);

        UPDATE_KEYS();
    }
}
