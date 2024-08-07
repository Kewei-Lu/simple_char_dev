#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/list.h>

#include "simple_chardev_debugfs_ops.h"
#include "simple_chardev_devfs_ops.h"

struct file_operations debug_fops = {
    .owner = THIS_MODULE,
    .open = debug_open,
    .release = debug_close,
    .read = msg_read,
    .write = msg_write,
    .unlocked_ioctl = NULL,
};

int debug_open(struct inode *, struct file *)
{
  int res = 0;
  pr_info("debugfs opened;\n");
  return res;
}

int debug_close(struct inode *, struct file *)
{
  int res = 0;
  pr_info("debugfs closed;\n");
  return res;
}

/* read all msgs stored in device */
ssize_t msg_read(struct file *fp, char *usr_buf, size_t count, loff_t *offset)
{
  int res = 0;
  int msg_count = 0;
  int i = 0;
  int formatted_str_size = 0;
  int copied_str_size = 0;
  int total_size_copied = 0;
  char **msg_arr = NULL; /* msg_arr is a string array consisting of all message body */
  k_msg *pos;
  // uint32_t msg_idx = 0;
  pr_info("msg read called;\n");
  pr_info("offset: %lld\n", *offset);

  list_for_each_entry(pos, &MSG_HEAD, list_node)
  {
    pr_info("pos addr: %p\n", pos);
    msg_count++;
  }
  pr_info("%d msg found; \n", msg_count);
  pr_info("count: %ld, offset: %lld; \n", count, *offset);

  if (0 == msg_count)
  {
    res = copy_to_user(usr_buf, NO_MSG_PROMPT, sizeof(NO_MSG_PROMPT));
    pr_info("copy_to_user res: %d; \n", res);
    // goto exit;
    if (*offset >= sizeof(NO_MSG_PROMPT))
      return 0;
    *offset = sizeof(NO_MSG_PROMPT);
    return sizeof(NO_MSG_PROMPT);
  }
  /* init msg_arr */
  msg_arr = vmalloc(sizeof(char *) * msg_count);
  if (NULL == msg_arr)
  {
    pr_err("msg_arr alloc failed ;\n");
    res = -EFAULT;
  }
  memset(msg_arr, 0, sizeof(char *) * msg_count);
  list_for_each_entry(pos, &MSG_HEAD, list_node)
  {
    // measure the size of formatted string
    formatted_str_size = snprintf(NULL, 0, "msg: %s\n", pos->u_msg->body);
    msg_arr[i] = vmalloc(sizeof(char) * formatted_str_size);
    if (NULL == msg_arr[i])
    {
      res = -EFAULT;
      pr_err("fail to allocate mem for %dth msg , ERRNO: %d;\n", i, res);
      goto exit;
    }
    copied_str_size = sprintf(msg_arr[i], "msg: %s\n", pos->u_msg->body);
    if (formatted_str_size != copied_str_size)
    {
      pr_err("formatted_str_size: %d, copied_str_size: %d; \n ", formatted_str_size, copied_str_size);
      res = -EFAULT;
      goto exit;
    };
    if (0 != (res = copy_to_user(usr_buf + total_size_copied, pos->u_msg->body, copied_str_size)))
    {
      pr_err("fail to copy msg struct from kernel to userspace, ERRNO: %d;\n", res);
      goto exit;
    }
    total_size_copied += copied_str_size;
    i++;
  }
  /* add tailing \0 */
  if (0 != (res = copy_to_user(usr_buf + total_size_copied, "\0", 1)))
  {
    pr_err("fail to copy tailing 0, ERRNO: %d;\n", res);
    goto exit;
  }
  //  *offset = total_size_copied;
  res = total_size_copied + 1;

  if (*offset >= res)
    res = 0;
  *offset = res;
exit:
  for (i = 0; i < msg_count; i++)
  {
    if (NULL != msg_arr[i])
      vfree(msg_arr[i]);
  }
  if (NULL != msg_arr)
    vfree(msg_arr);
  return res;
}

ssize_t msg_write(struct file *fp, const char *usr_buf, size_t count, loff_t *offset)
{
  int res = 0;
  msg *new_msg = NULL;
  k_msg *new_kmsg = NULL;
  char *new_msg_body = NULL;
  if (count == 0)
  {
    pr_info("read 0 bytes of data, ignore;\n");
    return res;
  }
  pr_info("read %d bytes of data;\n", count);
  new_msg = (msg *)vmalloc(sizeof(msg));
  new_msg_body = (unsigned char *)vmalloc(sizeof(unsigned char) * count);
  if (NULL == new_msg || NULL == new_msg_body)
  {
    res = -EFAULT;
    pr_err("fail to alloc mem for new_msg or new_msg_body;\n");
    goto exit;
  }
  if (0 != (res = copy_from_user(new_msg_body, usr_buf, count)))
  {
    res = -EINVAL;
    pr_err("fail to copy msg body from userspace to kernel, ERRNO: %d;\n", res);
    goto exit;
  }
  new_msg->msg_size = count;
  new_msg->body = new_msg_body;
  new_kmsg = vmalloc(sizeof(k_msg));
  if (NULL == new_kmsg)
  {
    res = -EINVAL;
    pr_err("fail to alloc mem for k_msg, err: %d\n", res);
    goto exit;
  }
  new_kmsg->u_msg = new_msg;
  list_add_tail(&new_kmsg->list_node, &MSG_HEAD);
  res = count;
  pr_info("res: %d\n", res);
exit:
  if (0 > res)
  {
    if (NULL != new_kmsg)
    {
      if (NULL != new_kmsg->u_msg)
      {
        if (NULL != new_kmsg->u_msg->body)
        {
          vfree(new_kmsg->u_msg->body);
          new_kmsg->u_msg->body = NULL;
        }
        vfree(new_kmsg->u_msg);
        new_kmsg->u_msg = NULL;
      }
      vfree(new_kmsg);
      new_msg_body = NULL;
    }
  }
  return res;
}
