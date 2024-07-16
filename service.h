typedef struct {
	int _device_major;
	struct gendisk *gd;
	struct block_device *base_bdev;
	char *base_device_name;
	char *virtual_device_name;
	char *base_device_path;
} dev;

void on_disk_close(dev *device);
