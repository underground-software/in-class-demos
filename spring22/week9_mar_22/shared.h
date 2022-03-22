#ifndef __LINUX_EXAMPLE_SHARED_H_
#define __LINUX_EXAMPLE_SHARED_H_
#include <linux/mutex.h>

struct shared {
	struct mutex rw_mutex;
	int shared_counter;
};

#endif//__LINUX_EXAMPLE_SHARED_H_
