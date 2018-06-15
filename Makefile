#  Makefile
#  Copyright (C) 2018 Ho Tuan Kiet
#  
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

GIT_SHA1        = $(shell git rev-parse --short HEAD)
TARGETS         = stm32/f0
DEVICE          = stm32f030f4p6
OPENCM3_DIR     = ./libopencm3/
OBJS            = main.o sha256.o logging.o mini_printf.o clock.o i2c.o address.o systick.o

COMMON          += -Wall -Wextra -pedantic -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--relax -DGIT_VERSION="\"$(GIT_SHA1)\""
CFLAGS          += -Os -std=c11 $(COMMON)
LDFLAGS         += -static -nostartfiles $(COMMON)
LDLIBS          += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

STFLASH         = st-flash
STFLASH_FLAGS   = --reset --format ihex

NM              = $(PREFIX)-nm

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
# Do not print "Entering directory ...".
MAKEFLAGS += --no-print-directory
endif

# Debug/production build
ifneq ($(PRODUCTION),1)
CFLAGS += -DDEBUG
endif

.PHONY: clean all lib upload size symbols-size
all: lib binary.elf binary.hex size

libopencm3_clean:
	$(Q)$(MAKE) -C $(OPENCM3_DIR) clean

clean: libopencm3_clean
	$(Q)$(RM) -rf binary.* *.o

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/gcc-config.mk

include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/gcc-rules.mk

lib:
	$(Q)if [ ! "`ls -A $(OPENCM3_DIR)`" ] ; then \
		printf "######## ERROR ########\n"; \
		printf "\tlibopencm3 is not initialized.\n"; \
		printf "\tPlease run:\n"; \
		printf "\t$$ git submodule init\n"; \
		printf "\t$$ git submodule update\n"; \
		printf "\tbefore running make.\n"; \
		printf "######## ERROR ########\n"; \
		exit 1; \
		fi
	$(Q)$(MAKE) TARGETS=$(TARGETS) -C $(OPENCM3_DIR)

size: binary.elf
	$(Q)$(SIZE) binary.elf

upload: binary.hex
	$(Q)$(STFLASH) $(STFLASH_FLAGS) write binary.hex

symbols-size: binary.elf
	$(Q)$(NM) -S --size-sort binary.elf -t dec | grep ' [TtDd] '
