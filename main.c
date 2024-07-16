#include <linux/blkdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

#include "service.h"

MODULE_AUTHOR("Viacheslav Sidorov");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BDD: Block Device Driver");

static dev device;

static int hwm_init(void) {
	pr_info("Init BDD\n");
	return 0;
}

static void hwm_exit(void) {
	on_disk_close(&device);
	pr_info("Exit BDD\n");
}

static void blk_submit_bio(struct bio *bio) {
	struct bio* clonned = bio_alloc_clone(device.base_bdev, bio, GFP_KERNEL, bio->bi_pool);

	if (!clonned) {
		pr_err("Error while bio clonning\n");
		return;
	}

	bio_chain(clonned, bio);
	submit_bio(clonned);
	bio_endio(bio);
	pr_info("Bio was submitted\n");
}

const struct block_device_operations md_fops = {
    .owner = THIS_MODULE,
    .submit_bio = blk_submit_bio,
};

/// param *arg : example : sda | vda | ...
static int create_disk(const char *arg, const struct kernel_param *kp) {
	// create base block device name
	if (device.base_device_name) {
		kfree(device.base_device_name);
	}
	device.base_device_name = kzalloc(strlen(arg) + 1, GFP_KERNEL);
	if (!device.base_device_name)
		return -ENOMEM;
	strcpy(device.base_device_name, arg);

	// create virtual block device name
	if (device.virtual_device_name) {
		kfree(device.virtual_device_name);
	}
	device.virtual_device_name = kzalloc(strlen(arg) + strlen("_virtual") + 1, GFP_KERNEL);
	if (!device.virtual_device_name)
		return -ENOMEM;
	sprintf(device.virtual_device_name, "%s_virtual", arg);

	// create base block device path
	if (device.base_device_path) {
		kfree(device.base_device_path);
	}
	device.base_device_path = kzalloc(strlen("/dev/") + strlen(arg) + 1, GFP_KERNEL);
	if (!device.base_device_path)
		return -ENOMEM;
	sprintf(device.base_device_path, "/dev/%s", arg);

	pr_info("%s %s %s\n", device.base_device_name, device.base_device_path,
		device.virtual_device_name);

	device._device_major = register_blkdev(0, device.virtual_device_name);
	if (device._device_major <= 0) {
		pr_err("Can't find device major number\n");
		return -ENXIO;
	}

	device.base_bdev =
	    blkdev_get_by_path(device.base_device_path, BLK_OPEN_READ | BLK_OPEN_WRITE, NULL, NULL);
	if (IS_ERR(device.base_bdev))
		pr_err("Opening error!\n");
	else
		pr_info("Was open?\n");

	device.gd = blk_alloc_disk(NUMA_NO_NODE);
	if (!device.gd) {
		pr_err("gd allocate error\n");
		return 0;
	} else {
		pr_info("allocate success\n");
	}

	sprintf(device.gd->disk_name, device.virtual_device_name);
	device.gd->major = device._device_major;
	device.gd->first_minor = 0;
	device.gd->minors = 16;
	device.gd->fops = &md_fops;
	set_capacity(device.gd, get_capacity(device.base_bdev->bd_disk));

	int err = add_disk(device.gd);
	pr_err("add_disk error code: %d\n", err);
	return 0;
}

static int remove_disk(const char *arg, const struct kernel_param *kp) {
	if (strcmp(device.base_device_name, arg) != 0) {
		pr_warn("Zero devices with this name was registred in this module\n");
	} else {
		on_disk_close(&device);
	}
	return 0;
}

static const struct kernel_param_ops create_disk_ops = {
    .set = create_disk,
    .get = NULL,
};

static const struct kernel_param_ops remove_disk_ops = {
    .set = remove_disk,
    .get = NULL,
};

MODULE_PARM_DESC(set_name, "Create blkdev by name");
module_param_cb(set_name, &create_disk_ops, NULL, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(unset_name, "Remove blkdev by name");
module_param_cb(unset_name, &remove_disk_ops, NULL, S_IRUGO | S_IWUSR);

module_init(hwm_init);
module_exit(hwm_exit);
