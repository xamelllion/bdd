#include "service.h"
#include <linux/blkdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

void bdd_on_disk_close(bdd_dev *device) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	if (device->base_bdev_handle) {
		bdev_release(device->base_bdev_handle);
		device->base_bdev_handle = NULL;
#else
	if (device->base_bdev) {
		blkdev_put(device->base_bdev, NULL);
		device->base_bdev = NULL;
#endif
		pr_info("Virtual block device was removed\n");
	}
	if (device->gd) {
		del_gendisk(device->gd);
		put_disk(device->gd);
		device->gd = NULL;
		pr_info("Gendisk of virtual block device was removed\n");
	}
	kfree(device->base_device_name);
	kfree(device->virtual_device_name);
	kfree(device->base_device_path);

	device->base_device_name = NULL;
	device->virtual_device_name = NULL;
	device->base_device_path = NULL;
}
