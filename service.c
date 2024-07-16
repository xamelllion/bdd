#include "service.h"
#include <linux/blkdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

void on_disk_close(dev *device) {
  if (device->gd) {
    del_gendisk(device->gd);
    device->gd = NULL;
    pr_info("Simple block device was removed\n");
  }
  if (device->_device_major) {
    unregister_blkdev(device->_device_major, device->virtual_device_name);
    device->_device_major = 0;
    pr_info("Simple block device was unregistred\n");
  }
  if (device->base_device_name) {
    kfree(device->base_device_name);
    device->base_device_name = NULL;
  }
  if (device->virtual_device_name) {
    kfree(device->virtual_device_name);
    device->base_device_name = NULL;
  }
  if (device->base_device_path) {
    kfree(device->base_device_path);
    device->base_device_name = NULL;
  }
}
