#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/minmax.h>
#include "shared.h"


enum drv_type {
	INC,
	DEC,
};

struct num {
	struct shared *shared_data;
	struct miscdevice mdev;
	enum drv_type Type;
};

static ssize_t num_read(struct file *file, char __user *out, size_t count, loff_t *f_pos) {
	struct num *ndev = container_of(file->private_data, struct num, mdev);
	size_t ret;
	size_t to_write;
	char buf[32];
	if (mutex_lock_interruptible(&ndev->shared_data->rw_mutex)) {
		return -ERESTARTSYS;
	}
	ret = snprintf(buf, sizeof(buf), "%i", ndev->shared_data->shared_counter);
	mutex_unlock(&ndev->shared_data->rw_mutex);
	to_write = min3(sizeof(buf), ret, count);
	if (copy_to_user(out, buf, to_write)) {
		return -EFAULT;
	}
	return to_write;
}

static ssize_t num_write(struct file *file, const char __user *in, size_t count, loff_t *f_pos) {
	struct num *ndev = container_of(file->private_data, struct num, mdev);
	if (mutex_lock_interruptible(&ndev->shared_data->rw_mutex)) {
		return -ERESTARTSYS;
	}
	switch(ndev->Type){
	case INC:
		ndev->shared_data->shared_counter++;
		break;
	case DEC:
		ndev->shared_data->shared_counter--;
	}
	mutex_unlock(&ndev->shared_data->rw_mutex);
	return 0;
}

static const struct file_operations num_fops = {
	.owner = THIS_MODULE,
	.read = num_read,
	.write = num_write,
};

static int num_probe(struct platform_device *pdev){
	int ret;
	struct num *ndev = devm_kzalloc(&pdev->dev, sizeof(struct num), GFP_KERNEL);

	if(!ndev){
		return -ENOMEM;
	}

	ndev->Type = platform_get_device_id(pdev)->driver_data;

	ndev->mdev = (struct miscdevice){
		.minor = MISC_DYNAMIC_MINOR,
		.name = ndev->Type == INC ? "num-inc" : "num-dec",
		.mode = 0666,
		.fops = &num_fops,
	};

	ndev->shared_data = dev_get_drvdata(&pdev->dev);

	ret = misc_register(&ndev->mdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not register num device\n");
		return ret;
	}

	ret = devm_add_action(&pdev->dev, (void *)misc_deregister, &ndev->mdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Cld not add misc device to devm\n");
		return ret;
	}

	dev_info(&pdev->dev,
	"num platform device registered with minor number %i\n",
	ndev->mdev.minor);

	return 0;
}

static const struct platform_device_id num_driver_ids[] = {
	{
		.name = "example-increment",
		.driver_data = INC,
	},
	{
		.name = "example-decrement",
		.driver_data = DEC,
	},
	{}
};

MODULE_DEVICE_TABLE(platform, num_driver_ids);

static struct platform_driver num_driver = {
	.id_table = num_driver_ids,
	.driver = {
		.name = "example-num",
	},
	.probe = num_probe,
};

module_platform_driver(num_driver);

MODULE_LICENSE("GPL");
