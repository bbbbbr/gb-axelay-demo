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

#include "entity_shots.h"



// TODO: optimize ship movement, probably fixed point and 1.5 pixels per frame
#define SHIP_MOVE_AMT_X 1u
#define SHIP_MOVE_AMT_Y 1u

#define SHIP_X_MIN (0u)
#define SHIP_X_MAX (DEVICE_SCREEN_PX_WIDTH - sprite_ship_PIVOT_W)

#define SHIP_Y_MIN (DEVICE_SCREEN_PX_HEIGHT * 3/5)
#define SHIP_Y_MAX (DEVICE_SCREEN_PX_HEIGHT - (sprite_ship_PIVOT_H / 2u))

#define SHIP_X_INIT (SHIP_X_MIN + ((SHIP_X_MAX - SHIP_X_MIN) / 2u))
#define SHIP_Y_INIT (SHIP_Y_MIN + ((SHIP_Y_MAX - SHIP_Y_MIN) / 2u))


#define SHIP_SPRITE_METASPR_COUNT 5u
#define SHIP_SPRITE_METASPR_MAX   (SHIP_SPRITE_METASPR_COUNT - 1u)
#define SHIP_SPRITE_SEL_BITSHIFT  2u
#define SHIP_SPRITE_SEL_MIN       (0u)
#define SHIP_SPRITE_SEL_MAX       (4u << SHIP_SPRITE_SEL_BITSHIFT)
#define SHIP_SPRITE_SEL_INIT      ((SHIP_SPRITE_SEL_MAX - SHIP_SPRITE_SEL_MIN) / 2)

// TODO: 16 bit movement
uint8_t ship_x, ship_y;
uint8_t ship_sprite_sel;
uint8_t ship_sprite_sel_temp;

void entity_ship_init(void) {
    ship_x = SHIP_X_INIT;
    ship_y = SHIP_Y_INIT;
}

void entity_ship_add(void) {
}

uint8_t entity_ship_update(uint8_t oam_high_water) {


    // TODO: map left/right panning
    // Horizontal movement
    if (KEY_PRESSED(J_LEFT)) {
        if (ship_x > SHIP_X_MIN) {
            ship_x -= SHIP_MOVE_AMT_X;
        }
        if (ship_sprite_sel > SHIP_SPRITE_SEL_MIN)
            ship_sprite_sel--;
    }
    else if (KEY_PRESSED(J_RIGHT)) {
        if (ship_x < SHIP_X_MAX) {
            ship_x += SHIP_MOVE_AMT_X;
        }
        if (ship_sprite_sel < SHIP_SPRITE_SEL_MAX)
            ship_sprite_sel++;
    } else {
        if (ship_sprite_sel > SHIP_SPRITE_SEL_INIT)
            ship_sprite_sel--;
        else if (ship_sprite_sel < SHIP_SPRITE_SEL_INIT)
            ship_sprite_sel++;
    }

    // Vertical Movement
    if (KEY_PRESSED(J_UP)) {
        if (ship_y > SHIP_Y_MIN) {
            ship_y -= SHIP_MOVE_AMT_Y;
        }
    }
    else if (KEY_PRESSED(J_DOWN)) {
        if (ship_y < SHIP_Y_MAX) {
            ship_y += SHIP_MOVE_AMT_Y;
        }
    }

    if (KEY_PRESSED(J_B)) {
        entity_shots_add(ship_x, ship_y);
    }


    // TODO: fixme
    if (oam_high_water < (40 - 7 - 1)) { // TODO: defines here

        // Move ship and update metasprite
        oam_high_water += move_metasprite(sprite_ship_metasprites[(ship_sprite_sel >> SHIP_SPRITE_SEL_BITSHIFT)],
                                         (SPR_TILES_START_SHIP),
                                         oam_high_water, ship_x, ship_y);

        // Canopy sprite on top of ship (1 x 8x16 sprite)
        oam_high_water += move_metasprite(sprite_ship_canopy_metasprites[(ship_sprite_sel >> SHIP_SPRITE_SEL_BITSHIFT)],
                                         (SPR_TILES_START_SHIP_CANOPY),
                                         oam_high_water, ship_x + 8, ship_y); // TODO DEFINE X OFFSET
    }

    return (oam_high_water);
}