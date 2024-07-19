typedef struct {
	int _device_major;
	struct gendisk *gd;
	struct block_device *base_bdev;
	struct bio_set* bs;
	char *base_device_name;
	char *virtual_device_name;
	char *base_device_path;
} bdd_dev;

void bdd_on_disk_close(bdd_dev *device);
