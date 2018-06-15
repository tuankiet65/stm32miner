/*
 *  sha256.h
 *  Copyright (C) 2018 Ho Tuan Kiet
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef SHA256_H
    #define SHA256_H

    #include <string.h>
    #include <inttypes.h>
    
    #include "logging.h"

    uint32_t scanhash_sha256d(uint32_t header[], uint32_t *result,
                              volatile uint8_t *new_data,
                              volatile uint32_t **nonce_ptr);

#endif
