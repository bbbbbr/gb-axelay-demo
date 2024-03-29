#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <gb/isr.h>

#include "map_scroll.h"
#include "../res/nesaxelay_background_map.h"

#include "common.h"

static uint8_t map_lcd_scy_start = 0;

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

    // 5. Closer, but maybe overdone
    //0, 15, 6, 5, 3, 3, 2, 3, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1, 0, -1, -1, 0, -1, 0, -1, -1, 0, -1, -1, -1, 0, -1, -1, 0, -1, -1, -1, 0, -1, -1, -1, 0, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 


   // TODO:
   //    - Maybe larger compressed horizon area at the top
   //  x - More severe spill over (done)

    // Merged from #5 & #1 together (5 at start, 1 at end), *** Plus dropping first entry in #5 ***
    // (144 - 32) = 112 scanlines 
    15,  6,  5,  3,  3,  2,  3,  2,
     1,  2,  2,  1,  1,  1,  1,  1,
     1,  1,  1,  0,  1,  1,  0,  1,
     1,  0,  1,  0,  1,  0,  1,  0,

     1,  0,  1,  0,  0,  1,  0,  1,
     0,  0,  1,  0,  0,  0,  1,  0,
     0,  0,  0,  1,  0,  0,  0,  0,
     // 0,  0,  0,  0,  1,  0,  0,  0,

     // 0,  0,  0, -1,  0,  0,  0,  0,
     // 0,  0,  0,  0, -1,  0,  0,  0,
     0,  0, -1,  0,  0,  0, -1,  0,
     0, -1,  0,  0, -1,  0,  0, -1,

     0,  0, -1,  0,  0, -1,  0, -1,
     0, -1,  0,  0, -1,  0, -1,  0,
    -1,  0, -1,  0, -1,  0, -1,  0,
    -1,  0, -1,  0, -1,  0, -1, -1,

     0, -1,  0, -1,  0, -1,  0, -1,
    //-1,  0, -1,  0, -1,  0, -1, -1,
    // 0,  0,  0

/*  // Full 144 Scanline LUT
    15,  6,  5,  3,  3,  2,  3,  2,
     1,  2,  2,  1,  1,  1,  1,  1,
     1,  1,  1,  0,  1,  1,  0,  1,
     1,  0,  1,  0,  1,  0,  1,  0,

     1,  0,  1,  0,  0,  1,  0,  1,
     0,  0,  1,  0,  0,  0,  1,  0,
     0,  0,  0,  1,  0,  0,  0,  0,
     0,  0,  0,  0,  1,  0,  0,  0,

     0,  0,  0, -1,  0,  0,  0,  0,
     0,  0,  0,  0, -1,  0,  0,  0,
     0,  0, -1,  0,  0,  0, -1,  0,
     0, -1,  0,  0, -1,  0,  0, -1,

     0,  0, -1,  0,  0, -1,  0, -1,
     0, -1,  0,  0, -1,  0, -1,  0,
    -1,  0, -1,  0, -1,  0, -1,  0,
    -1,  0, -1,  0, -1,  0, -1, -1,

     0, -1,  0, -1,  0, -1,  0, -1,
    -1,  0, -1,  0, -1,  0, -1, -1,
     0,  0,  0
*/
};


// LYC and HBlank scanline STAT handler
// * First Scanline :
//   - Starts at line HORIZON_Y_START
//   - Load new Scroll Y start value for map with offset
//   - STAT_REG
//     - Turn *ON* HBlank interrupt
//     - Turn *OFF** LYC interrupt
//
// * Map Scrolling Area: Per Scanline compress/stretch effect
//   - Apply SCY offsets based on LY based LUT
void map_fx_stat_isr(void) __interrupt __naked {
    __asm \

    // Get current Y line and determine whether to do
    // LYC interrupt or scanline interrupt
    push af
    ldh a, (_LY_REG+0)
    sub a, #HORIZON_Y_START
    jr  z, first_scanline_of_stretch_setup$        // Less cycles when not taken, use that path for HBlank

    // For all later scanlines, apply the warp effect
    //
    // Entered during start of ~HBlank
    scanline_map_stretch$:
        push hl

        ld  l, a                        // A = (current scanline - top non-warped region size)
        ld  h, #0x7E                    // High byte of address for SCY offsets LUT (fixed location 256 byte aligned)
        ldh a, (_SCY_REG+0)             // Get current Scroll Y
        add a, (hl)                     // Add LUT offset to Scroll Y based indexed based on current LYC value
        ldh (_SCY_REG+0), a             // Apply the updated scroll value

        pop hl
        pop af
        reti;

    // Do some setup on first scanline of the warped region
    // This only gets called *ONCE* per frame
    //
    // Entered during ~oam scan
    first_scanline_of_stretch_setup$:
        // Do the busy loop for HBlank before turning on the HBlank ISR
        // That way the ISR doesn't accidentally re-trigger
        mode_wait_loop$:
        ldh a, (_STAT_REG + 0)          // Wait to exit until mode change (3 -> 0)
        and a, #STATF_BUSY              // so LY changes and this handler doesn't get called twice after HBlank ISR is enabled
        jr nz, mode_wait_loop$          //

        ld  a, #STATF_MODE00            // Turn on HBlank interrupt for rest of frame (also turns off LYC in, but it's optional)
        ldh (_STAT_REG+0), a            //

                                        // Below Needs to happen after HBlank wait loop in order to show mid-line glitching from the changeover

        ld  a, (#_map_lcd_scy_start)                // Reset Scroll Y for start of warped region
        add a, #HORIZON_Y_START     // Offset to compensate for vertical size of top non-warped region
        ldh (_SCY_REG + 0), a

        ld  a, (#_map_x)                // Update Scroll X for warped region
        ldh (_SCX_REG + 0), a

        ldh a, (_LCDC_REG + 0)
        and a, #LCDCF_BG9C00 ^ #0xFF    // Turn off horizon BG Map and select main one
        ldh (_LCDC_REG + 0), a          // Timing here is sensitive to which mode in order to avoid glitches

        // TODO: try prepping it so that the first call to the HBlank handler can happen immediately from here

        pop af
        reti

    __endasm;

}

// // Register scanline / STAT ISR handler function for the STAT interrupt
ISR_VECTOR(VECTOR_STAT, map_fx_stat_isr)



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
    map_lcd_scy_start = (uint8_t)map_y; // Clip to 0 - 255 for hardware map buffer height

    // Switch to alternate BG map with horizon
    LCDC_REG |= LCDCF_BG9C00;

    // Turn OFF HBlank int (STATF_MODE00) and turn on LYC int (STATF_LYC)
    STAT_REG = STATF_LYC;
    IF_REG &= ~0x02; // TODO CONSTANT // Clear any pending stat interrupt, fixes junk line at top of screen


    // Horizontal background map scrolling updaates
    if (draw_queued_map) {
        // Draw a chunk of the currently queued row, increments Vram destination and map source each time

        // TODO: OPTIMIZE: it's just 4 bytes, this could be optimized if variable chunk sizing is removed
        // In VBlank, memcpy should be ok here
        // set_data(
        memcpy(p_vram_dest, // Y Row clamped to HW map buffer dimensions (32 x 32) then x 32 ( << 5) to get row address in vram
               p_map_src,  // Map Offset: Map Y downshifted to tiles, clamped to map Height (0x80 in tiles)
               (nesaxelay_background_map_WIDTH >> 3) / MAP_SCROLL_CHUNK_COUNT);  // Write: 1 x row of tiles / bytes

        if (_cpu == CGB_TYPE) {
            // Draw map tile colors/etc
            VBK_REG = 1; // Same as setting tiles above, but with tile attributes
            // Draw a chunk of the currently queued row

            // In VBlank, memcpy should be ok here
            // set_data(
            memcpy(p_vram_dest, // Y Row clamped to HW map buffer dimensions (32 x 32) then x 32 ( << 5) to get row address in vram
                   p_map_attr_src,  // Map Offset: Map Y downshifted to tiles, clamped to map Height (0x80 in tiles)
                   (nesaxelay_background_map_WIDTH >> 3) / MAP_SCROLL_CHUNK_COUNT);  // Write: 1 x row of tiles / bytes
            VBK_REG = 0; // Return to writing tile IDs

            p_map_attr_src += (nesaxelay_background_map_WIDTH >> 3) / 8;
        }

        // move to next VRAM update chunk address
        p_vram_dest += (nesaxelay_background_map_WIDTH >> 3) / 8;
        p_map_src += (nesaxelay_background_map_WIDTH >> 3) / 8;
        draw_queued_map--;
    }
}
// TODO: Does above need to preserve A REG?


void map_fx_isr_enable(void) {

    // Enable STAT ISR
    __critical {
        STAT_REG = STATF_LYC; // First pass after turning on uses LYC STAT interrupt
        STAT_REG = STATF_MODE00;
        add_VBL(vblank_isr_map_reset);
        LYC_REG = HORIZON_Y_START;
    }
    // Try to wait until just after the start of the next frame
    wait_vbl_done();
    set_interrupts(IE_REG | LCD_IFLAG);
}


void map_fx_isr_disable(void) {

    // Disable STAT ISR
    __critical {
        set_interrupts(IE_REG & ~LCD_IFLAG);
        remove_VBL(vblank_isr_map_reset);
    }
}





