#include <string.h>
#include "table.h"

uint32_t keys[TABLE_ENTRIES];
uint8_t vals[TABLE_ENTRIES];

void reset_table(void) {
    // 0 means missing data
    memset(&keys, 0, TABLE_ENTRIES * sizeof(keys[0]));
    memset(&vals, 0, TABLE_ENTRIES * sizeof(vals[0]));
}
