
#ifndef __map_h_INCLUDE
#define __map_h_INCLUDE


// TODO: convert to 16 bit
// create stripped 8 bit version for ISR

extern uint16_t map_y;
extern uint8_t  map_x;
extern uint8_t  map_x_top;

void map_isr_enable(void);

#endif