#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/minmax.h>
#include "shared.h"

enum driver_type {
	INC,
	DEC,
};


struct platform_num {
	struct shared_data *shared_data;
	struct miscdevice mdev;
	enum driver_type type;
};


static ssize_t num_read(struct file *file, char __user *out, size_t count, loff_t *f_pos) {
	struct platform_num *num = container_of(file->private_data, struct platform_num, mdev);
	size_t ret;
	size_t to_write;
	char buf[32];
	if (mutex_lock_interruptible(&num->shared_data->rw_mutex))
		return -ERESTARTSYS;

	ret = snprintf(buf, sizeof(buf), "%i", num->shared_data->count);
	mutex_unlock(&num->shared_data->rw_mutex);

	to_write = min3(sizeof(buf), ret, count);
	if (copy_to_user(out, buf, to_write))
		return -EFAULT;

	return to_write;
}

static ssize_t num_write(struct file *file, const char __user *in, size_t count, loff_t *f_pos) {
	struct platform_num *num = container_of(file->private_data, struct platform_num, mdev);
	if (mutex_lock_interruptible(&num->shared_data->rw_mutex))
		return -ERESTARTSYS;

	switch(num->type) {
	case INC:
		++num->shared_data->count;
		break;
	case DEC:
		--num->shared_data->count;
	}
	mutex_unlock(&num->shared_data->rw_mutex);
	return count;
}

static const struct file_operations num_fops = {
	.owner = THIS_MODULE,
	.read = num_read,
	.write = num_write,
};

static int num_probe(struct platform_device *pdev) {
	int ret;
	struct shared_data *shared = platform_get_drvdata(pdev);
	enum driver_type type = platform_get_device_id(pdev)->driver_data;
	struct platform_num *num = devm_kzalloc(&pdev->dev, sizeof(*num), GFP_KERNEL);
	dev_info(&pdev->dev, "probe called");
	dev_info(&pdev->dev, "this was the probe for %s\n", type==INC ? "increment" : "decrement");
	if (!num) {
		dev_err(&pdev->dev, "Could not allocate num device");
		return -ENOMEM;
	}
	num->shared_data = shared;
	num->type = type;
	num->mdev = (struct miscdevice) {
		.minor = MISC_DYNAMIC_MINOR,
		.name = type == INC ? "num-inc" : "num-dec",
		.mode = 0666,
		.fops = &num_fops,
	};

	ret = misc_register(&num->mdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not register num device");
		return ret;
	}

	ret = devm_add_action(&pdev->dev, (void *)misc_deregister, &num->mdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not add misc device to devm");
		return ret;
	}

	return 0;
}

static int num_remove(struct platform_device *pdev) {
	enum driver_type type = platform_get_device_id(pdev)->driver_data;
	dev_info(&pdev->dev, "remove called");
	dev_info(&pdev->dev, "this was the remove for %s\n", type==INC ? "increment" : "decrement");
	return 0;
}

static struct platform_device_id num_driver_id[] = {
	{ .name = "example-increment", .driver_data = INC, },
	{ .name = "example-decrement", .driver_data = DEC, },
	{},
};

static struct platform_driver num_driver = {
	.id_table = num_driver_id,
	.driver = {
		.name = "example-platform-num",
	},
	.probe = num_probe,
	.remove = num_remove,
};

module_platform_driver(num_driver);

MODULE_DESCRIPTION("platform driver for platform device and devm architecture example");
MODULE_AUTHOR("Daniel Bauman");
MODULE_LICENSE("GPL");
