
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

#include "../res/nesaxelay_background_map.h"

#include "../res/horizon_map.h"
#include "../res/horizon_tiles.h"
#include "../res/sprite_boulders.h"
#include "../res/sprite_ship.h"
#include "../res/sprite_ship_canopy.h"
#include "../res/sprite_shots.h"

#include "input.h"
#include "common.h"

#include "map_fx.h"
#include "map_scroll.h"
#include "player_input.h"
#include "entity_boulders.h"
#include "entity_ship.h"
#include "entity_shots.h"


// TODO: move to gfx.x
void init_gfx_map() {

    set_bkg_data(0, nesaxelay_background_map_TILE_COUNT, nesaxelay_background_map_tiles);

    if (_cpu == CGB_TYPE) {
        // Set CGB Palette
        set_bkg_palette(0, nesaxelay_background_map_PALETTE_COUNT, nesaxelay_background_map_palettes);
    } else {
        // Set DMG palette
        BGP_REG = DMG_PALETTE(DMG_BLACK, DMG_DARK_GRAY, DMG_LITE_GRAY, DMG_WHITE);
    }

    // Set up horizon on alternate map (usually used for the Window)
    set_bkg_data(245u, horizon_tiles_count, horizon_tiles);
    set_win_submap(0, 0, 32, 4, horizon_map, horizon_map_width);
}


void init_gfx_sprites() {

    // Load metasprite tile data into VRAM
    set_sprite_data((SPR_TILES_START_BOULDERS), sprite_boulders_TILE_COUNT, sprite_boulders_tiles);
    set_sprite_data((SPR_TILES_START_SHIP), sprite_ship_TILE_COUNT, sprite_ship_tiles);
    set_sprite_data((SPR_TILES_START_SHIP_CANOPY), sprite_ship_canopy_TILE_COUNT, sprite_ship_canopy_tiles);
    set_sprite_data((SPR_TILES_START_SHOTS), sprite_shots_TILE_COUNT, sprite_shots_tiles);

    SPRITES_8x16;

    if (_cpu == CGB_TYPE) {
        // Set CGB Palette
        set_sprite_palette(0, nesaxelay_background_map_PALETTE_COUNT, nesaxelay_background_map_palettes);
    } else {
        // Set DMG palette (top two colors BLACK for higher contrast against background)
        OBP0_REG = DMG_PALETTE(DMG_BLACK, DMG_BLACK, DMG_LITE_GRAY, DMG_WHITE);
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
    entity_ship_init();

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
        oam_high_water = entity_ship_update(oam_high_water);
        oam_high_water = entity_shots_update(oam_high_water);

        // Hide rest of the hardware sprites, because amount of sprites differ between animation frames.
        hide_sprites_range(oam_high_water, 40);

        UPDATE_KEYS();
    }
}
