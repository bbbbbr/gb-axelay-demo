#ifndef entity_shots_H
#define entity_shots_H

// #define ENTITY_COUNT_SHOTS  5u // Lots of slowdown with this
#define ENTITY_COUNT_SHOTS  2u

extern uint8_t shots_active_total;
extern entity shots[];

void entity_shots_init(void);
void entity_shots_add(uint8_t, uint8_t);
uint8_t entity_shots_update(uint8_t);

#endif