#include <gbdk/platform.h>
#include <stdint.h>
#include "input.h"

#include <gb/isr.h>


// How to...
//
// * Efficiently turn on the HBlank ISR part way down the frame
//   -  while still having a fixed ISR jump
//      - and without having a LYC vs HBlank test every time the ISR fires?
//        - Could have a massive VBlank ISR at the start of the frame that
//          doesn't exit until around the expected line?


// Start of New Frame
//
// * Top of Screen Area
//   - Reset SCY_REG to Zero
//   - Update SCX_REG based on ???
//   - Select Alternate BG Map
//   - Calculate new Map Y Scroll and save it (Support Map X Scrolling too?)
//   - STAT_REG
//     - Turn *OFF* HBlank interrupt
//     - Turn *ON** LYC interrupt
//
// * Map Scrolling Area: Start (Line: __?__)
//   - Load new SCY_REG starting value for map
//   - STAT_REG
//     - Turn *OFF* HBlank interrupt
//     - Turn *ON** LYC interrupt
//
// * Map Scrolling Area: Per Scanline
//   - Load SCY
//   - Load SCY Offset Value based on current LY
//   - Adjust SCY based on Offset


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

    // Merged from #5 & #1 together (5 at start, 1 at end), *** Plus dropping first entry in #5 ***
    15, 6, 5, 3, 3, 2, 3, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, 0, -1, 0, -1, -1,
    0, 0, 0
};

const uint8_t * p_scy_horizon_offsets;

// // Serial link interrupt handler
void map_stat_isr(void) __interrupt __naked {

    __asm \
        push HL \
        push AF \
        ldh a, (_LY_REG+0)    // Get current Y Line
        ld  l, a \
        ld  h, #0x7E          // High byte of address for SCY offsets LUT (fixed location 256 byte aligned)
        ldh a, (_SCY_REG+0)   // Get current Scroll Y
        add a, (hl)           // Add LUT offset to Scroll Y based indexed based on current LYC value
        ldh (_SCY_REG+0), a   // Apply the updated scroll value
        pop AF \
        pop HL \
        reti;
    __endasm;
}

// // Register the STAT ISR handler function for the STAT interrupt
 ISR_VECTOR(VECTOR_STAT, map_stat_isr)


void map_isr_enable(void) {

    // Enable STAT ISR
    __critical {
        STAT_REG = (STATF_MODE01 | STATF_MODE00); /// TODO CONSTANT
    }
    set_interrupts(IE_REG | LCD_IFLAG);
}


void map_isr_disable(void) {

    // Disable STAT ISR
//    __critical {
        set_interrupts(IE_REG & ~LCD_IFLAG);
//    }
}



void map_init(void) {

}


