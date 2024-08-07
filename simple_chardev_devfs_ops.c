#include <linux/fs.h>
#include <linux/export.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/module.h>

#include "simple_chardev_devfs_ops.h"

// MODULE_AUTHOR("Kewei Lu");
// MODULE_LICENSE("Dual BSD/GPL");

const struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_close,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
};

int dev_open(struct inode *, struct file *)
{
    int res = 0;
    pr_info("dev opened;\n");
    return res;
}

int dev_close(struct inode *, struct file *)
{
    int res = 0;
    pr_info("dev closed;\n");
    return res;
}

ssize_t dev_read(struct file *, char *, size_t, loff_t *)
{
    int res = 0;
    pr_info("read is called; \n");
    return res;
}
ssize_t dev_write(struct file *, const char *, size_t, loff_t *)
{
    int res = 0;
    pr_info("write is called; \n");
    return res;
}

long dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    int res;
    pr_info("ioctl get called;\n");
    switch (cmd)
    {
    case K_CMD_SEND_MSG:
    {
        if (0 != (res = dev_handle_send_msg(arg)))
        {
            pr_err("dev_handle_send_msg fail;\n");
            return -EINVAL;
        };
        break;
    }
    case K_CMD_GET_MSG_LEN:
    {
        if (0 != (res = dev_handle_get_msg_len(arg)))
        {
            pr_err("dev_handle_get_msg_len fail;\n");
            return -EINVAL;
        };
        break;
    }
    case K_CMD_GET_MSG:
    {
        if (0 != (res = dev_handle_get_msg_by_id(arg)))
        {
            pr_err("dev_handle_get_msg_by_id fail;\n");
            return -EINVAL;
        };
        break;
    }
    break;
    default:
        pr_err("Unknown ioctl cmd: %d", cmd);
        res = -EINVAL;
    };
    return res;
}

/**
 * handler for receiving the message sent from user space
 * @input: arg represents the struct msg
 */
int dev_handle_send_msg(unsigned long arg)
{
    int res = 0;
    msg *new_msg = NULL;
    // uint32_t *new_msg_idx = (uint32_t *)vmalloc(sizeof(uint32_t));
    // uint32_t *new_msg_size = (uint32_t *)vmalloc(sizeof(uint32_t));
    k_msg *new_k_msg = NULL;
    k_msg *pos = NULL;
    // char *new_msg_body = NULL;
    // // char *tmp_msg_body = NULL; // since we do not know the actual length of msg body for the first time, we use this as an intermediate buffer
    // pr_info("dev_handle_send_msg get called\n");
    new_msg = (msg *)arg;
    // if (!access_ok(arg, sizeof(msg)))
    // {
    //     res = -EFAULT;
    //     pr_err("user space memory not accessible;\n");
    //     goto exit;
    // }
    // new_msg = (msg *)vmalloc(sizeof(msg));
    // if (0 != (res = copy_from_user(new_msg, (msg *)arg, sizeof(msg))))
    // {
    //     res = -EINVAL;
    //     pr_err("fail to copy msg struct from userspace to kernel, ERRNO: %d;\n", res);
    //     goto exit;
    // }
    // new_msg_body = (unsigned char *)vmalloc(sizeof(unsigned char) * (new_msg->msg_size));
    // if (NULL == new_msg_body)
    // {
    //     res = -EFAULT;
    //     pr_err("fail to alloc mem for new msg body;\n");
    //     goto exit;
    // }
    // // pr_info("arg addr: %p\n", (void *)arg);
    // // new_msg->idx = ((msg *)arg)->idx;
    // // new_msg->msg_size = ((msg *)arg)->msg_size;
    // if (0 != (res = copy_from_user(new_msg_body, new_msg->body, new_msg->msg_size)))
    // {
    //     res = -EINVAL;
    //     pr_err("fail to copy msg body from userspace to kernel, ERRNO: %d;\n", res);
    //     goto exit;
    // }
    // new_msg->body = new_msg_body;
    pr_info("copy to k_msg;\n");
    new_k_msg = vmalloc(sizeof(k_msg));
    if (NULL == new_k_msg)
    {
        res = -EFAULT;
        pr_err("fail to alloc mem for new k_msg;\n");
        goto exit;
    }
    new_k_msg->u_msg = new_msg;
    pr_info("handle send  umsg addr %p;\n", new_msg);
    pr_info("handle send body addr:  %p;\n", new_k_msg->u_msg->body);
    pr_info("handle send body msg:  %s;\n", new_k_msg->u_msg->body);
    pr_info("add tail;\n");

    list_add_tail(&new_k_msg->list_node, &MSG_HEAD);
    list_for_each_entry(pos, &MSG_HEAD, list_node)
    {
        // msg_idx = pos->u_msg->idx;
        // measure the size of formatted string
        pr_info("pos addr %p;\n", pos);
        pr_info("umsg addr %p;\n", pos->u_msg);
        pr_info("body addr %p;\n", pos->u_msg->body);
        pr_info("done ;\n");
    }
exit:
    if (res != 0)
    // err occur, clean mem
    {
        if (NULL != new_k_msg)
        {
            vfree(new_k_msg);
            new_k_msg = NULL;
        }
    }

    return res;
}

int dev_handle_get_msg_len(unsigned long arg)
{
    k_msg *pos;
    int res = 0;
    uint32_t count = 0;
    /* user should pass a int32 variable */
    if (!access_ok(arg, sizeof(uint32_t)))
    {
        pr_err("user space memory not accessible;\n");
        return -EFAULT;
    }
    list_for_each_entry(pos, &MSG_HEAD, list_node)
    {
        count++;
    }
    if (0 != (res = copy_to_user((uint32_t *)arg, &count, sizeof(uint32_t))))
    {
        pr_err("fail to copy msg arr length from kernel to userspace, ERRNO: %d;\n", res);
        return -EINVAL;
    }
    return 0;
}

int dev_handle_get_msg_by_id(unsigned long arg)
{
    k_msg *pos;
    int res = 0;
    msg *user_msg = (msg *)vmalloc(sizeof(msg));
    if (NULL == user_msg)
    {
        pr_err("user_msg NULL;\n");
        res = -EINVAL;
        goto exit;
    }
    /* user should pass a msg struct with idx filled*/
    if (!access_ok(arg, sizeof(msg)))
    {
        pr_err("userspace memory not accessible;\n");
        res = -EFAULT;
        goto exit;
    }
    if (0 != (res = copy_from_user(user_msg, (msg *)arg, sizeof(msg))))
    {
        pr_err("fail to copy from userspace to kernel;\n");
        res = -EFAULT;
        goto exit;
    };
    pr_err("do not support;\n");
    // list_for_each_entry(pos, &MSG_HEAD, list_node)
    // {
    //     if (user_msg->idx == pos->u_msg->idx)
    //     {
    //         user_msg->msg_size = pos->u_msg->msg_size;
    //         user_msg->body = pos->u_msg->body;
    //         /**
    //          *  copy msg back to userspace
    //          *  userspace should prepare an allocated buffer
    //          */
    //         if (0 != (res = copy_to_user(((msg *)arg)->body, pos->u_msg->body, pos->u_msg->msg_size)))
    //         {
    //             pr_err("fail to copy msg struct from kernel to userspace, ERRNO: %d;\n", res);
    //             goto exit;
    //         }
    //         // if (0 != (res = copy_to_user(((msg *)arg)->msg_size, pos->u_msg->msg_size, sizeof(pos->u_msg->msg_size))))
    //         //     pr_err("fail to copy msg struct from kernel to userspace, ERRNO: %d;\n", res);
    //         break;
    //     }
    // }
exit:
    if (NULL != user_msg)
        vfree(user_msg);
    return res;
}

static int clean_msg(void)
{
    k_msg *pos, *n_pos;
    int res = 0;
    pr_info("clean mem for msgs;\n");
    list_for_each_entry_safe(pos, n_pos, &MSG_HEAD, list_node)
    {
        if (pos->u_msg != NULL)
        {
            if (pos->u_msg->body != NULL)
                vfree(pos->u_msg->body);
            vfree(pos->u_msg);
        }
        list_del(&pos->list_node);
    }
    pr_info("clean mem for msgs finished;\n");
    return res;
}
