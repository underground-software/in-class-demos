#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");



static dev_t hello_device;
static struct cdev *hello_cdev;

static int hello_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Hello Module: open\n");
	return 0;
}

static int hello_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Hello Module: release\n");
	return 0;
}

static ssize_t hello_read(struct file *file, char __user *data, size_t count, loff_t *f_pos)
{
	int ret;
	size_t i;
	const char str[] = "HELLO";
	printk(KERN_INFO "Hello Module: read\n");

	for(i = 0; i < count; ++i)
	{
		ret = copy_to_user(data+i, str + (i%5), 1);
		if(ret)
		{
			printk(KERN_ERR "Hello Module: bad read! %i\n", ret);
			return ret;
		}
	}
	*f_pos += count;
	return count;
}

static const struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.release = hello_release,
	.read = hello_read,
};

static int hello_init(void) {
	int ret;
	printk(KERN_INFO "Hello Module: init\n");
	ret = alloc_chrdev_region(&hello_device, 0, 1, "hello");
	if (ret < 0) {
		printk(KERN_ERR "Hello Module: unnable to allocate region %i\n", ret);
		goto fail_region;
	}

	hello_cdev = cdev_alloc();
	if (!hello_cdev) {
		ret = -ENOMEM;
		printk(KERN_ERR "Hello Module: unable to allocate char dev %i\n", ret);
		goto fail_cdev;
	}

	cdev_init(hello_cdev, &hello_fops);

	ret = cdev_add(hello_cdev, hello_device, 1);
	if (ret < 0) {
		printk(KERN_ERR "Hello Module: unable to add char dev %i\n", ret);
		goto fail_add;
	}

	printk(KERN_INFO "Hello Module: got major # %i got minor # %i\n", MAJOR(hello_device), MINOR(hello_device));

	return 0;

fail_add:
	cdev_del(hello_cdev);
fail_cdev:
	unregister_chrdev_region(hello_device, 1);
fail_region:
	return ret;
}

static void hello_exit(void) {
	printk(KERN_INFO "Hello Module: exit\n");
	cdev_del(hello_cdev);
	unregister_chrdev_region(hello_device, 1);
}
module_init(hello_init);
module_exit(hello_exit);
