#ifndef __SIMPLE_CHARDEV_DEBUGFS__
#define __SIMPLE_CHARDEV_DEBUGFS__

#define NO_MSG_PROMPT "no msg\n"

extern struct file_operations debug_fops;
ssize_t msg_read(struct file *fp, char *usr_buf, size_t count, loff_t *offset);
ssize_t msg_write(struct file *fp, const char *usr_buf, size_t count, loff_t *offset);

#endif /* __SIMPLE_CHARDEV_DEBUGFS__ */
