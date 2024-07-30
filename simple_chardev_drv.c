
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/printk.h>
#include <linux/debugfs.h>

#include "simple_chardev_devfs_ops.h"
#include "simple_chardev_debugfs_ops.h"

MODULE_AUTHOR("Kewei Lu");
MODULE_LICENSE("Dual BSD/GPL");

#define CHR_DEV_FIRST_MIN 0
#define CHR_DEV_CNT 1
#define CHR_DEV_NAME "simple_chardev"

static dev_t my_dId;
static struct cdev *my_cdev;
static struct device *my_udev; // userspace device interface
static struct class *my_class;

static int init_char_dev(void)
{
  int res = 0;
  struct dentry *file;
  pr_info("init simple_chardev;\n");
  /**
   * allocate dev id
   * we do not have real devices, so use dynamic
   */
  if (0 != (res = alloc_chrdev_region(&my_dId, CHR_DEV_FIRST_MIN, CHR_DEV_CNT, CHR_DEV_NAME)))
  {
    pr_err("fail to alloc chardev devid;\n");
    return -EINVAL;
  }

  /**
   * init struct cdev
   * cdev is used in kernel space
   */
  my_cdev = cdev_alloc();
  if (NULL == my_cdev)
  {
    pr_err("fail to init struct cdev;\n");
    return -EINVAL;
  }
  cdev_init(my_cdev, &dev_fops);
  /* add dev */
  if (0 != (res = cdev_add(my_cdev, my_dId, CHR_DEV_CNT)))
  {
    pr_err("fail to init cdev; res: %d\n", res);
    return -EINVAL;
  }

  /* expose to userspace devfs */
  my_class = class_create(THIS_MODULE, CHR_DEV_NAME);
  if (NULL == (my_udev = device_create(my_class, NULL, my_dId, NULL, "%s", CHR_DEV_NAME)))
  {
    pr_err("fail to create userspace cdev interface;\n");
    return -EINVAL;
  };

  /* expose to debug fs */
  if (NULL == (file = debugfs_create_file(CHR_DEV_NAME, 0644, NULL, NULL, &debug_fops)))
  {
    pr_err("fail to create userspace debugfs interface;\n");
    return -EINVAL;
  }

  return res;
}

static void finish_char_dev(void)
{
  if (NULL == my_cdev)
  {
    pr_info("cdev: NULL;\n");
  }
  else
  {
    cdev_del(my_cdev);
  }
  if (NULL == my_udev)
  {
    pr_info("my_udev: NULL;\n");
  }
  else
  {
    device_destroy(my_class, my_dId);
  }
  class_destroy(my_class);
  unregister_chrdev_region(my_dId, CHR_DEV_CNT);
  pr_info("CHR_DEV_NAME finished;\n");
}

module_init(init_char_dev);
module_exit(finish_char_dev);
