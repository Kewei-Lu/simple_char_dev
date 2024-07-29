#ifndef __SIMPLE_CHARDEV_DEVFS__

#define __SIMPLE_CHARDEV_DEVFS__
#include <linux/kernel.h>
#include <asm-generic/ioctl.h>

/**
 * Common utils section
 */

// #define pr_prefix "kewei-ko: "
// #define pr_info(args...) printk(pr_prefix args)
// #define pr_err(args...) printk(pr_prefix args)

#define CONFIG_PAGE_OFFSETUL 0xC0000000 // should I have it here manually?

/**
 * MSG
 */
#define MAX_MSG_SIZE 1024
extern struct list_head MSG_HEAD;

int dev_open(struct inode *, struct file *);
int dev_close(struct inode *, struct file *);
ssize_t dev_read(struct file *, char *, size_t, loff_t *);
ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
long dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg);
int dev_handle_send_msg(unsigned long arg);
int dev_handle_get_msg_len(unsigned long arg);
int dev_handle_get_msg_by_id(unsigned long arg);

typedef struct msg_s
{
  uint32_t idx;
  uint32_t msg_size;
  unsigned char *body;
} msg;

typedef struct k_msg_s
{
  msg *u_msg;
  struct list_head list_node;
} k_msg;

/**
 * IOCTL CMD NRs
 */

#define K_MAGIC_NR ('k')
#define K_CMD_NR_SEND_MSG (0)
#define K_CMD_NR_GET_MSG_LEN (1)
#define K_CMD_NR_GET_MSG (2)

#define K_CMD_SEND_MSG \
  _IOW(K_MAGIC_NR, K_CMD_NR_SEND_MSG, msg)

#define K_CMD_GET_MSG_LEN \
  _IOR(K_MAGIC_NR, K_CMD_NR_GET_MSG_LEN, uint32_t)

#define K_CMD_GET_MSG \
  _IOR(K_MAGIC_NR, K_CMD_NR_GET_MSG, msg)

extern const struct file_operations dev_fops;

#endif /* __SIMPLE_CHARDEV_DEVFS__ */