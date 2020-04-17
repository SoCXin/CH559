import usb.core

BEGIN_ADDR = 0x0000

dev = usb.core.find(idVendor=0x4348)

# get info

dev.write(0x02, bytearray([0xA7, 0, 0, 0x1F, 0]))
msg = dev.read(0x82, 64)

uuid = msg[22:26]
print('UUID = %02X%02X%02X%02X' % (uuid[0], uuid[1], uuid[2], uuid[3]))

uuid_sum = sum(uuid) & 0xFF

# set checksum

dev.write(0x02, bytearray([0xA3, 31, 0] + [uuid_sum for i in range(28)]))
msg = dev.read(0x82, 64, 100)
chip_id = msg[4]

if chip_id != 0x58 and chip_id != 0x59:
    print('Wrong CHIP_ID')
    exit()

# dump data

print('Starting dump ...')

flash = bytearray([0xFF for i in range(0xFC08)])
seq = [0xFF] + [i for i in range(255)] # optimized for 0xFF and 0x00
for addr in reversed(range(BEGIN_ADDR, 0xFC00)):
    print('\rAddress: 0x%04X' % addr, end='')

    data = flash[addr:addr+8]
    data[7] ^= chip_id

    flag = False
    for val in seq:
        data[0] = val
        # check current byte is correct
        dev.write(0x02, bytearray([0xA6, 13, 0, addr & 0xFF, addr >> 8, 0, 0, 0]) + data)
        msg = dev.read(0x82, 64)
        if msg[4] == 0:
            flash[addr] = val
            flag = True
            break
    
    if flag == False:
        print('\nOops! Something must be wrong ...')
        exit()

with open(r'dump.bin', 'wb') as f:
    f.write(flash[BEGIN_ADDR:0xFC00])

print('\nDump successfully')
