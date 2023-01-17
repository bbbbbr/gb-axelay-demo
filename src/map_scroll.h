
#ifndef map_scroll_H
#define map_scroll_H

extern uint16_t map_y;
extern uint8_t  map_x;
extern uint8_t  map_x_top;

extern uint8_t draw_queued_map;
extern uint8_t * p_vram_dest;
extern const uint8_t * p_map_src;
extern const uint8_t * p_map_attr_src;

#define MAP_SCROLL_CHUNK_COUNT 8

void map_scroll_init(void);
void map_scroll_update(void);
void map_scroll_redraw_all(void);

#endif