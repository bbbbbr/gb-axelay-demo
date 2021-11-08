#include <gbdk/platform.h>
#include <stdint.h>
#include "input.h"

#include <gb/isr.h>

extern uint8_t map_x, map_y;
extern uint8_t map_x_top;

// How to...
//
// * Efficiently turn on the HBlank ISR part way down the frame
//   -  while still having a fixed ISR jump
//      - and without having a LYC vs HBlank test every time the ISR fires?
//        - Could have a massive VBlank ISR at the start of the frame that
//          doesn't exit until around the expected line?


// Start of New Frame
//
// 1. VBlank:            Reset Map for Top screen area,      Use Alt  BG Map, LY Interrupt: ON, HBlank Int: OFF
// 2. LY Interrupt:      Reset Map for BG Scrolling/Warping, Use Main BG Map, LY Interrupt: OFF, HBlank Int: ON
// 3. HBlank Interrupts: Update Map BG Y Scroll from LUT



// One offset per scanline, added to SCY value of preceding line
//
// (See spreadsheet for calculation)
//
// Calculated for:
// * Scanlines 0 - 143
// * Using Hardware Map 0 - 247
// TODO: convert these to twos complement instead of using conversion overflow?
// TODO: Improve fixed addressing allocation
const uint8_t __at(0x7E00) scy_horizon_offsets[] = {
    // 1. Not Bad, but not prounoucned enough, sort of 45 degree view
    //  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, -1

    // 2. Near opposite of what is wanted, but wrong at the top
    // 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 0, -1, -1, 0, -1, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 

    // 3. It's sort of closer, but wrong at the top and upside down
    // 0, -3, -3, -2, -3, -2, -3, -2, -3, -3, -2, -3, -2, -3, -2, -3, -2, -3, -3, -2, -3, -2, -3, -2, -3, -2, -3, -2, -3, -2, -3, -2, -3, -2, -3, -2, -3, -2, -2, -3, -2, -3, -2, -3, -2, -2, -3, -2, -2, -3, -2, -2, -3, -2, -2, -3, -2, -2, -3, -2, -2, -2, -3, -2, -2, -2, -2, -2, -3, -2, -2, -2, -2, -2, -2, -2, -2, -3, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2, -2, -1, -2, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -1, -2, -1, -2, -1, -2, -1, -2, -1, -1, -2, -1, -1, -2, -1, -1, -2, -1, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 

    // 4. A little better, but wrong at the top still
    // 0, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, 

    // 5. Closer, but maybe overdone
    //0, 15, 6, 5, 3, 3, 2, 3, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, -1, 0, -1, 0, -1, -1, 0, -1, -1, -1, 0, -1, -1, 0, -1, -1, -1, 0, -1, -1, -1, 0, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 


// Padding for initial non-warped area????  (TODO: remove and fix code to not require?)

// TODO: compress this so that bottom area stretches more to compensate for non-fullscreen

    // Merged from #5 & #1 together (5 at start, 1 at end), *** Plus dropping first entry in #5 ***
    15, 6, 5, 3, 3, 2, 3, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, -1,
    0, 0, 0
};


#define WARPED_AREA_START_Y 32

// TODO: would it be better to swap out some variable and then do a reset jump?

// LYC and HBlank scanline STAT handler
// * First Scanline :
//   - Starts at line WARPED_AREA_START_Y
//   - Load new SCY_REG starting value for map
//     - Adjust based on offset from top of screen
//   - STAT_REG
//     - Turn *ON* HBlank interrupt
//     - Turn *OFF** LYC interrupt
//
// * Map Scrolling Area: Per Scanline
//   - Apply SCY offset based on LY based LUT
void map_stat_isr(void) __interrupt __naked {
    __asm \
        push AF \
        ldh a, (_LY_REG+0)            // Get current Y Line
        sub a, #WARPED_AREA_START_Y \
        jp  NZ, scanline_map_stretch$ // If it's not the first line then kkip to effect

    // Do some setup on first scanline of the warped region
    // This only gets called *ONCE* per frame
    first_scanline_of_stretch_setup$:

        // TODO: change map

        ld  a, (#_map_y)               // Reset Scroll Y for start of warped region
        add a, #WARPED_AREA_START_Y    // TODO: for now, compensate for top non-warped region size
        ldh (_SCY_REG + 0), a \
        ld  a, (#_map_x)               // Update Scroll X for warped region
        ldh (_SCX_REG + 0), a \

        ld  a, #STATF_MODE00           // Turn on HBlank interrupt for rest of frame (also turns off LYC in, but it's optional)
        ldh (_STAT_REG+0), a \
        pop AF \
        reti \

    // For all later scanlines, apply the warp effect
    scanline_map_stretch$:
        push HL \
        ld  l, a \            // A = (current scanline - top non-warped region size)
        ld  h, #0x7E          // High byte of address for SCY offsets LUT (fixed location 256 byte aligned)
        ldh a, (_SCY_REG+0)   // Get current Scroll Y
        add a, (hl)           // Add LUT offset to Scroll Y based indexed based on current LYC value
        ldh (_SCY_REG+0), a   // Apply the updated scroll value
        pop HL \
        pop AF \
        reti;
    __endasm;

}

// // Register scanline / STAT ISR handler function for the STAT interrupt
ISR_VECTOR(VECTOR_STAT, map_stat_isr)



// * Top of Screen Area
//   - Reset SCY_REG to Zero
//   - Update SCX_REG based on map_x_top
//   - Select Alternate BG Map
//   - ???? Calculate new Map Y Scroll and save it (Support Map X Scrolling too?)
//   - STAT_REG
//     - Turn *OFF* HBlank interrupt
//     - Turn *ON** LYC interrupt
//
void vblank_isr_map_reset (void) {
    // Updates for TOP non-warped region
    SCY_REG = 0;
    SCX_REG = map_x_top;

    STAT_REG = STATF_LYC; // Turn OFF HBlank int (STATF_MODE00) and turn on LYC int (STATF_LYC)
}
// TODO: Does above need to preserve A REG?


void map_isr_enable(void) {

    // Enable STAT ISR
    __critical {
        STAT_REG = STATF_LYC; // First pass after turning on uses LYC STAT interrupt
        STAT_REG = STATF_MODE00;
        add_VBL(vblank_isr_map_reset);
        LYC_REG = WARPED_AREA_START_Y;
    }
    // Try to wait until just after the start of the next frame
    wait_vbl_done();
    set_interrupts(IE_REG | LCD_IFLAG);
}


void map_isr_disable(void) {

    // Disable STAT ISR
    __critical {
        set_interrupts(IE_REG & ~LCD_IFLAG);
        remove_VBL(vblank_isr_map_reset);
    }
}



void map_init(void) {

}


