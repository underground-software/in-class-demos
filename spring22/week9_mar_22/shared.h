#ifndef __LINUX_EXAMPLE_SHARED_H_
#define __LINUX_EXAMPLE_SHARED_H_
#include <linux/mutex.h>

struct shared_data {
	int count;
	struct mutex rw_mutex;
};

#endif//__LINUX_EXAMPLE_SHARED_H_
