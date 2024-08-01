# SPDX-License-Identifier: GPL-2.0-only

from pathlib import Path
import subprocess
import random

class Colors:
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

disk_size = 1024*1024
block_size = 4096

# sequential of random numbers represents 4K file filled of them
rainbow = [x for x in range(1, 10)]
random.shuffle(rainbow)

start_sector = random.randint(0, block_size) // 512
rainbow_sectors = [
    x//512 for x in range(start_sector, disk_size-block_size, block_size)
]
random.shuffle(rainbow_sectors)

# sequential of sectors for writing
rainbow_sectors = rainbow_sectors[0:9]

Path('/tmp/bdd/').mkdir(exist_ok=True)

# create 1.txt..9.txt files filled with numbers 1..9
for x in range(1, 10):
    with open(f'/tmp/bdd/{x}.txt', 'w') as f:
        f.write(str(x)*block_size)

# write files into disk to 'rainbow_sectors'
for i in range(len(rainbow)):
    seek = rainbow_sectors[i] // 8
    subprocess.call(
        f'dd of=/dev/sda_virtual if=/tmp/bdd/{rainbow[i]}.txt oflag=direct block_size=4K count=1 seek={seek}'.split()
    )
    print(f"{Colors.BLUE}Write color '{rainbow[i]}' to sector '{seek*8}'{Colors.ENDC}")

# read all written data from actual disk
subprocess.call(
    'dd if=/dev/sda of=/tmp/bdd/dump.txt iflag=direct block_size=4K count=9 skip=0'.split()
)

# check invariant of log-structuring
# numbers in 'dump.txt' MUST be in the same order as in 'rainbow'
with open('/tmp/bdd/dump.txt') as f:
    dump_str = f.read()
    if len(dump_str)//block_size != 9:
        print(f'{Colors.FAIL}Error: dump size {len(dump_str)//block_size} mismatch actual written size 9{Colors.ENDC}')
        exit()
    for x in range(0, len(dump_str), block_size):
        if int(dump_str[x]) != rainbow[x//block_size]:
            print(f'{Colors.FAIL}Error: address shuffling{Colors.ENDC}')
            exit()

print(f'{Colors.GREEN}Successful test{Colors.ENDC}')
