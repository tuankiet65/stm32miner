#ifndef SHA256_H
    #define SHA256_H

    #include <string.h>
    #include <inttypes.h>
    
    #include "logging.h"

    uint32_t scanhash_sha256d(uint32_t header[], uint32_t *result, volatile uint8_t *new_data);

#endif
