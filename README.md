# STM32Miner

## Overview
STM32Miner is a project for making a Bitcoin mining board consisted of microcontrollers. A board consists of a master, which receives work from an external source, then splits it into multiple jobs which will then be sent to worker MCUs.

This is the source code of the firmware running on the worker MCUs.

## Why?
I figured out this is the best way to learn how to develop firmwares on the STM32 line.

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

The firmware exposes itself as a xx byte register with the following structure:

First address | Last address | type | Name | Purpose | RW/RO
--------------|--------------|------|------|--------|------
0x00 | 0x00 | `uint8_t` | `new_job_id` | Job ID of the new job | RW
0x01 | 0x50 | `uint32_t[20]` | `new_header` | Header of the new job | RW
0x51 | 0x51 | `uint8_t` | `execute_job` | Write any non-zero value to start mining on the new job | RW
0x52 | 0x52 | `uint8_t` | `force_calibration` | _(reserved)_ Write non-zero value to force the calibration of the High Speed Internal clock | RW
0x53 | 0x?? | `char[8]` | `version` | Firmware's git commit | RO
0x?? | 0x?? | `uint32_t` | `hashrate` | Current hashrate | RO
0x?? | 0x?? | `uint8_t` | `current_job_id` | Job ID of the currently mining job | RO
0x?? | 0x?? | `uint8_t` | `finished` | Non-zero if the current job | RO
0x?? | 0x?? | `uint32_t` | `winning_nonce` | Winning nonce (aka the nonce that meets the requirement) | RO

To initiate a mining job:
* Write the job ID to `new_job_id`
* Write the 80-byte header to `new_header`
* Write any non-zero value to `execute_job`. The firmware will start mining the new job immediately. When it starts the `current_job_id` will change to the ID of the started job.

To retrieve the result of a mining job:
* Wait until `finished` contains non-zero value
* Fetch the nonce in `winning_nonce`. This is the nonce that causes the block's SHA256d hash to contain at least 32 leading zeros (aka the hash's leading 4 bytes are zero)
