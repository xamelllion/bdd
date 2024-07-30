name = bdd
ip   = 192.168.122.186

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	rsync -havuz -e ssh ./ root@$(ip):/driver/

ins:
	insmod $(name).ko

rm:
	rmmod $(name)

echo_set:
	echo -n "sda" >  /sys/module/$(name)/parameters/set_name

echo_unset:
	echo -n "sda" >  /sys/module/$(name)/parameters/unset_name

ddw:
	head -c 24000 /dev/random > /tmp/rdata.txt
	dd of=/dev/sda_virtual if=/tmp/rdata.txt iflag=direct bs=4K count=1 seek=0

ddr:
	dd if=/dev/sda_virtual of=/tmp/wdata.txt iflag=direct bs=4K count=1 skip=0
