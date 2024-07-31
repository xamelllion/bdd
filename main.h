/* SPDX-License-Identifier: GPL-2.0-only */

#pragma once

#include "hashmap.h"

typedef struct {
	int _device_major;
	struct gendisk *gd;
	struct bdev_handle *base_bdev_handle;
	struct bio_set *bs;
	int last_unused_sector;
	hashmap *map;
	char *base_device_name;
	char *virtual_device_name;
	char *base_device_path;
} bdd_dev;

void bdd_on_disk_close(bdd_dev *device);
