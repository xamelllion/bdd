#include <linux/version.h>

typedef struct {
	int _device_major;
	struct gendisk *gd;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	struct bdev_handle *base_bdev_handle;
#else
	struct block_device *base_bdev;
#endif
	char *base_device_name;
	char *virtual_device_name;
	char *base_device_path;
} bdd_dev;

void bdd_on_disk_close(bdd_dev *device);
