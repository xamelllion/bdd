name = bdd
ip   = 192.168.122.72
obj-m += $(name).o
$(name)-objs += ./main.o ./service.o


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
