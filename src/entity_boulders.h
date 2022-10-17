#ifndef entity_boulders_H
#define entity_boulders_H


#define ENTITY_COUNT_BOULDERS 8u
// #define ENTITY_COUNT_BOULDERS 6u

// extern uint8_t boulders_col_group_y_count[PARTITION_COUNT];
// extern uint8_t boulders_col_group_y[PARTITION_COUNT][ENTITY_COUNT_BOULDERS]; // TODO: per-partition bucket size can probably be reduced by 1/2 at least
extern uint8_t boulders_col_group_x[PARTITION_COUNT];

extern entity boulders[];

void entity_boulders_init(void);

uint8_t entity_boulders_update(uint8_t);

#endif