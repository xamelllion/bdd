## Driver for Log-structured data storage
Linux kernel 6.8+

### Build
```bash
make
```

### Install
```bash
make ins
```

### Remove
```bash
make rm
```

### Test
```bash
# build and install
make && make ins

# setup virtual dick
# for 'sda' driver will create 'sda_virtual' disk
echo -n "sda" > /sys/module/$(name)/parameters/set_name

# execute test
python3 ./scripts/test.py

# remove module from system
make rm
```
