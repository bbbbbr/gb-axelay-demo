
#ifndef map_scroll_H
#define map_scroll_H

extern uint16_t map_y;
extern uint8_t  map_x;
extern uint8_t  map_x_top;

void map_scroll_init(void);
void map_scroll_update(void);
void map_scroll_redraw_all(void);

#endif