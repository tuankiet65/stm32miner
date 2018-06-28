# This script is meant to be run on MicroPython-supported boards
# with the slave's I2C pins connected to I2C port 1
# (the I2C port number is different per boards so look it out yourself)
# https://micropython.org/

import pyb
import ustruct

def test(quick = True):
    header = [0x20, 0x00, 0x00, 0x02,
    
              0x16, 0x96, 0x8a, 0xc6, 0xd6, 0x61, 0x32, 0xe0, 0x9a, 0x56, 0x88, 0x79, 0xc4, 0xe7, 0x18, 0x20,
              0x55, 0x67, 0xdc, 0x79, 0x00, 0xf0, 0x61, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             
              0x1f, 0x4c, 0xbd, 0x24, 0xc2, 0x9e, 0x1b, 0xde, 0x69, 0x60, 0x57, 0xa8, 0xc6, 0x35, 0xc4, 0x5f,
              0x1d, 0xf1, 0xbe, 0xdf, 0x9a, 0xe6, 0x69, 0x04, 0x0b, 0x0b, 0x02, 0xc0, 0x83, 0xf5, 0xfd, 0xf4,
            
              0x58, 0x55, 0x9f, 0x70,
             
              0x18, 0x03, 0x8b, 0x85,
             
              0x00, 0x00, 0x37, 0x67]
    addr = None
              
    i2c = pyb.I2C(1, baudrate=100000)
    i2c.init(pyb.I2C.MASTER)
    
    if quick:
        print('quick testing')
    else:
        print('very slow testing')
        header[-2] = 0
        header[-1] = 0x30

    print("scanning for i2c address")
    i2c_addresses = i2c.scan()
    i2c_addresses.remove(25)
    i2c_addresses.remove(30)
    if (len(i2c_addresses) == 1):
        addr = i2c_addresses[0]
        print("address: 0x{:02x}".format(addr))
    else:
        print("cannot found")
        return

    version = i2c.mem_read(7, addr, 0x00).decode("ascii")
    print("version: {}".format(version))

    print("checking state")
    state = i2c.mem_read(1, addr, 0x08)[0]
    if (state not in [0x01, 0x03, 0x04, 0x02]):
        print("invalid state, state is 0x{:02x}".format(state))
        return;
    if (state == 0x02):
        print("canceling previous job")

    print('writing header')
    i2c.mem_write(0x50, addr, 0x12)
    i2c.mem_write(bytes(header), addr, 0x13)
    i2c.mem_write(0xff, addr, 0x63)
    
    print('waiting for winning nonce...')
    for i in range(1, 10000):
        pyb.delay(3000)
        state = i2c.mem_read(1, addr, 0x08)[0]
        hashrate = ustruct.unpack("<I", bytes(i2c.mem_read(4, addr, 0x09)))[0]
        if (state != 0x03):
            print('still not finished, hashrate: {} count: {}...'.format(hashrate, i))
        else:
            nonce_raw = i2c.mem_read(4, addr, 0x0e)
            nonce = ustruct.unpack(">I", bytes(nonce_raw))[0]
            print('received nonce is 0x{:08x}'.format(nonce))
            if nonce == 0x25e93767:
                print('correct nonce, test passed')
            else:
                print('incorrect nonce, test failed')
            return
    print('test failed')
