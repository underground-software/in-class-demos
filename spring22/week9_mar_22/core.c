#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "shared.h"

static struct platform_device *inc, *dec;
static struct shared *shared_data;

static int hello_init(void) {
	int ret = -ENOMEM;
	printk(KERN_INFO "Hello Module: init\n");

	shared_data = kzalloc(sizeof(*shared_data), GFP_KERNEL);
	if (!shared_data)
		goto fail_alloc;

	mutex_init(&shared_data->rw_mutex);

	inc = platform_device_alloc("example-increment", PLATFORM_DEVID_AUTO);
	if (!inc)
		goto fail_inc;

	dec = platform_device_alloc("example-decrement", PLATFORM_DEVID_AUTO);
	if (!dec)
		goto fail_dec;

	platform_set_drvdata(inc, shared_data);
	platform_set_drvdata(dec, shared_data);

	ret = platform_device_add(dec);
	if (ret)
		goto fail_add_dec;

	ret = platform_device_add(inc);
	if (ret)
	{
		platform_device_unregister(dec);
		goto fail_dec;
	}

	return 0;

fail_add_dec:
	platform_device_put(dec);
fail_dec:
	platform_device_put(inc);
fail_inc:
	kfree(shared_data);
fail_alloc:
	return ret;
}

static void hello_exit(void) {
	printk(KERN_INFO "Hello Module: exit\n");
	platform_device_unregister(dec);
	platform_device_unregister(inc);
	kfree(shared_data);
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_DESCRIPTION("core driver for platform device and devm architecture example");
MODULE_AUTHOR("Charles Mirabile");
MODULE_LICENSE("GPL");
