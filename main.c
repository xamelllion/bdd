#include <linux/blkdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "service.h"

#define BDD_MODULE_NAME "bdd"

MODULE_AUTHOR("Viacheslav Sidorov");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BDD: Block Device Driver");

static bdd_dev device;

static int bdd_init(void)
{
	int err;
	device._device_major = register_blkdev(0, BDD_MODULE_NAME);
	if (device._device_major <= 0) {
		pr_err("Can't find device major number\n");
		return -ENXIO;
	}
	device.bs = kzalloc(sizeof(struct bio_set), GFP_KERNEL);
	if (!device.bs)
		return -ENOMEM;
	err = bioset_init(device.bs, BIO_POOL_SIZE, 0, BIOSET_NEED_BVECS);
	if (err)
		goto interrupt;
	pr_info("Init BDD\n");
	return 0;

interrupt:
	kfree(device.bs);
	unregister_blkdev(device._device_major, BDD_MODULE_NAME);
	pr_err("Error in bioset_init\n");
	return -ENOMEM;
}

static void bdd_exit(void)
{
	bdd_on_disk_close(&device);
	if (device._device_major) {
		unregister_blkdev(device._device_major, BDD_MODULE_NAME);
		device._device_major = 0;
		pr_info("Virtual block device was unregistred\n");
	}
	pr_info("Exit BDD\n");
}

static void bdd_bio_end_io(struct bio *bio)
{
	bio_endio(bio->bi_private);
	bio_put(bio);
}

static void bdd_submit_bio(struct bio *bio)
{
	struct bio* clone = bio_alloc_clone(device.base_bdev_handle->bdev, bio, GFP_KERNEL, device.bs);

	if (!clone) {
		pr_err("Error while bio clonning\n");
		return;
	}

	clone->bi_private = bio;
	clone->bi_end_io = bdd_bio_end_io;
	submit_bio(clone);
	pr_info("Bio was submitted\n");
}

const struct block_device_operations bdd_fops = {
    .owner = THIS_MODULE,
    .submit_bio = bdd_submit_bio,
};

/// param *arg : example : sda | vda | ...
static int create_disk(const char *arg, const struct kernel_param *kp)
{
	// create base block device name
	kfree(device.base_device_name);
	device.base_device_name = kzalloc(strlen(arg) + 1, GFP_KERNEL);
	if (!device.base_device_name)
		return -ENOMEM;
	strcpy(device.base_device_name, arg);

	// create virtual block device name
	kfree(device.virtual_device_name);
	device.virtual_device_name = kzalloc(strlen(arg) + strlen("_virtual") + 1, GFP_KERNEL);
	if (!device.virtual_device_name)
		return -ENOMEM;
	sprintf(device.virtual_device_name, "%s_virtual", arg);

	// create base block device path
	kfree(device.base_device_path);
	device.base_device_path = kzalloc(strlen("/dev/") + strlen(arg) + 1, GFP_KERNEL);
	if (!device.base_device_path)
		return -ENOMEM;
	sprintf(device.base_device_path, "/dev/%s", arg);

	device.base_bdev_handle =
	    bdev_open_by_path(device.base_device_path, BLK_OPEN_READ | BLK_OPEN_WRITE, NULL, NULL);

	if (IS_ERR(device.base_bdev_handle))
		pr_err("Error while oppening base device\n");
	else
		pr_info("Base device successfully openned\n");

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
	device.gd->fops = &bdd_fops;
	set_capacity(device.gd, get_capacity(device.base_bdev_handle->bdev->bd_disk));

	int err = add_disk(device.gd);
	pr_err("add_disk error code: %d\n", err);
	return 0;
}

static int remove_disk(const char *arg, const struct kernel_param *kp)
{
	if (strcmp(device.base_device_name, arg) != 0)
		pr_warn("Zero devices with this name was registred in this module\n");
	else
		bdd_on_disk_close(&device);
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

module_init(bdd_init);
module_exit(bdd_exit);
