# STM32Miner

## Overview
STM32Miner is a project for making a Bitcoin mining board consisted of microcontrollers. A board consists of a master, which receives work from an external source, then splits it into multiple jobs which will then be sent to worker MCUs.

This is the source code of the firmware running on the worker MCUs.

## Why?
I figure out this is the best way to get started with programming on the STM32 and ARM Cortex-M uC.

## Compatibility
Currently, this firmware is only knows to work on the [STM32F030F4P6](http://www.st.com/en/microcontrollers/stm32f030f4.html).

The firmware contains platform-dependant stuff like RCC, UART, I2C which is not compatible with other STM32 and ARM Cortex-M parts. It _might_ be compatible with the STM32F2/3/4 line, but definitely not the STM32F1/not STM32 line.

## Building and installing

### Flavours
You can either make a debug build or a production build:
* Production build: Fat (uses nearly all the available Flash space), fast (nearly 6khash/s), doesn't print any debug messages over UART. ***Recommended***.
* Debug build: Small (about 8k), slow (as SHA-256 loops are not unrolled), prints a lot of debug messages over UART at 921600 baud.

### Building
Prerequisite:
* General stuff: make, git, python
* ARM specific: arm-none-eabi-gcc, arm-none-eabi-binutils, arm-none-eabi-newlib
* STM32 specific: [stlink](https://github.com/texane/stlink)

On Arch Linux, use the following command to install them:

    pacman -S make git arm-none-eabi-gcc arm-none-eabi-binutils arm-none-eabi-newlib python stlink

Then clone this repository and initialize its submodules:

    git clone https://gitlab.com/tuankiet65/stm32miner
    cd stm32miner
    git submodule init
    git submodule update

Then build it:

    make lib
    make PRODUCTION=1 all # if production build
    make PRODUCTION=0 all # if debug build

### Installing
Connect your STM32F030F4P6 to your computer via a ST-Link v2 programmer, then run:

    make upload

to upload the firmware to the microcontrollers.

## Usage
All communication with the master is done using I2C:
* PA9: SCL
* PA10: SDA

The firmware exposes itself as a register bank with the following structure:

Address | Name | Size (in bytes) | Type | Description | RW/RO
--------|------|-----------------|------|-------------|------
0x00 | `version` | 8 | `char[8]` | Firmware's git commit | RO
0x08 | `state` | 1 | `uint8_t` | Current state of the miner | RO
0x09 | `hashrate` | 4 | `uint32_t` | Current hashrate | RO
0x0d | `current_job_id` | 1 | `uint8_t` | Job ID of the current job` | RO
0x0e | `winning_nonce` | 4 | `uint32_t` | Current hashrate | RO
0x12 | `new_job_id` | 1 | `uint8_t` | New job ID | RW
0x13 | `new_header` | 80 | `uint32_t[20]` | New block header | RW
0x63 | `execute_job` | 1 | `uint8_t` | Write non-zero value to mine the new block header | RW

`state` is defined as:
```cpp
#define STATE_READY 0x01
#define STATE_WORKING 0x02
#define STATE_FOUND 0x03
#define STATE_NOT_FOUND 0x04
#define STATE_ERROR 0xff
```

To initiate a mining job:
* Write the job ID to `new_job_id`
* Write the 80-byte header to `new_header`
* Write any non-zero value to `execute_job`. The firmware will start mining the new block header immediately. When it starts the `current_job_id` will change to the ID of the started job.

To retrieve the result of a mining job:
* Poll `finished` until it contains a non-zero value
* Fetch the nonce in `winning_nonce`. This is the nonce that causes the block's SHA256d hash to contain at least 32 leading zeros (aka the leading 4 bytes are 0x00000000)
