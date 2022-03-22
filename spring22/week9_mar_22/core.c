#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static int hello_init(void) {
	int ret = -ENOMEM;
	struct platform_device *child;
	printk(KERN_INFO "Hello Module: init\n");
	child = platform_device_alloc("example-char-device", PLATFORM_DEVID_AUTO);
	if (!child)
		goto fail_alloc;

	ret = platform_device_add(child);
	if (ret)
		goto fail_add;

	return 0;

fail_add:
	platform_device_put(child);
fail_alloc:
	return ret;
}

static void hello_exit(void) {
	printk(KERN_INFO "Hello Module: exit\n");
	cdev_del(hello_cdev);
	unregister_chrdev_region(hello_device, 1);
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_DESCRIPTION("core driver for platform device and devm architecture example");
MODULE_AUTHOR("Charles Mirabile");
MODULE_LICENSE("GPL");
