#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define BOARDSIZE 42

static dev_t tic_dev;
static struct cdev *tic_cdev;

static char *grid;
static struct mutex rw;
char c;

static int tic_open(struct inode *inode, struct file *file) {
	printk(KERN_INFO "Tic Tac Toe opened\n");
	return 0;
}

static int tic_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "Tic Tac Toe released\n");
	return 0;
}

static ssize_t tic_read(struct file *file, char __user *data, size_t count, loff_t *f_pos){
	int i, j;
	int l = 0;
	int ret = 0;
	char buf[BOARDSIZE];
	size_t bytesToWrite;

	printk(KERN_INFO "Tic Tac Toe read called\n");
	if (mutex_lock_interruptible(&rw)) {
		printk(KERN_ERR "Tic Tac Toe Read: failed to acquire access to grid\n");
		return -ERESTARTSYS;
	}
	for (i = 0; i < 3; i++) {
		buf[l++] = '|';
		for(j = 0; j < 3; j++) {
			buf[l++] = ' ';
			buf[l++] = grid[i*3 + j];
			buf[l++] = ' ';
			buf[l++] = '|';
		}
		buf[l++] = '\n';
	}
	bytesToWrite = ((*f_pos + count) > BOARDSIZE) ? (BOARDSIZE - *f_pos) : count;
	if (copy_to_user(data, *f_pos + buf, bytesToWrite)) {
		ret = -EFAULT;
		goto out;
	}
	*f_pos += bytesToWrite;
	ret = bytesToWrite;
out:
	mutex_unlock(&rw);
	return ret;
}

static ssize_t tic_write(struct file *file, const char __user *data, size_t count, loff_t *f_pos){
	int ret = 0;
	int i;
	char buf[3];
	int row, col;
	printk(KERN_INFO "Tic Tac Toe write called with %i characters\n", (int)count);
	if (count < 3) {
		printk(KERN_ERR "Tic Tac Toe write: bad format, should be \"row,column\" (e.g. \"1,3\")");
		return -EINVAL;
	}
	if (mutex_lock_interruptible(&rw)) {
		printk(KERN_ERR "Tic Tac Toe write: failed to acquire access to grid\n");
		return -ERESTARTSYS;
	}
	if(copy_from_user(buf, data, 3)){
		ret = -EFAULT;
		printk(KERN_ERR "Tic Tac Toe write: failed to copy data from user\n");
		goto out;
	}
	printk(KERN_INFO "%c%c%c\n", buf[0], buf[1], buf[2]);
	switch (buf[0]) {
	case '1':
		row = 0;
		break;
	case '2':
		row = 1;
		break;
	case '3':
		row = 2;
		break;
	case '0':
		row = 9;
		break;
	default:
		ret = -EINVAL;
		goto out;
	}
	switch (buf[2]) {
	case '1':
		col = 0;
		break;
	case '2':
		col = 1;
		break;
	case '3':
		col = 2;
		break;
	case '0':
		col = 9;
		break;
	default:
		ret = -EINVAL;
		goto out;
	}
	if (row == 9 || col == 9) {
		for (i = 0; i < 9; i++) {
			grid[i] = ' ';
		}
		c = 'X';
		ret = count;
		goto out;
	}
	if (grid[row*3 + col] != ' ') {
		printk(KERN_ERR "Tic Tac Toe write: square is already used.\n");
		ret = -EINVAL;
		goto out;
	}
	grid[row*3 + col] = c;
	c = (c == 'X') ? 'O' : 'X';
	ret = count;
out:
	mutex_unlock(&rw);
	return ret;
}

static const struct file_operations tic_fops = {
	.owner = THIS_MODULE,
	.open = tic_open,
	.release = tic_release,
	.read = tic_read,
	.write = tic_write,
};

static int tic_init(void) {
	int ret = 0;
	int i;
	printk(KERN_INFO "Tic Tac Toe init called\n");
	ret = alloc_chrdev_region(&tic_dev, 0, 1, "tictactoe");
	if (ret < 0) {
		printk(KERN_ERR "Tic Tac Toe init: unnable to allocate region %i\n", ret);
		goto fail_region;
	}

	tic_cdev = cdev_alloc();
	if (!tic_cdev) {
		ret = -ENOMEM;
		printk(KERN_ERR "Tic Tac Toe init: unnable to allocate char dev %i\n", ret);
		goto fail_cdev;
	}

	cdev_init(tic_cdev, &tic_fops);

	ret = cdev_add(tic_cdev, tic_dev, 1);
	if (ret < 0) {
		printk(KERN_ERR "Tic Tac Toe init: unnable to add char dev %i\n", ret);
		goto fail_add;
	}

	mutex_init(&rw);

	if (mutex_lock_interruptible(&rw)) {
		ret = -ERESTARTSYS;
		printk(KERN_ERR "Tic Tac Toe init: Failed to acquire access to grid\n");
		goto fail_add;
	}

	grid = (char *) kzalloc(9, GFP_KERNEL);
	if (!grid) {
		printk(KERN_ERR "Tic Tac Toe open: failed to allocate memory for grid\n");
		ret = -ENOMEM;
		goto fail_mutex;
	}

	for (i = 0; i < 9; i++){
		grid[i] = ' ';
	}

	c = 'X';

	mutex_unlock(&rw);

	printk(KERN_INFO "Tic Tac Toe init: got major # %i minor: %i\n", MAJOR(tic_dev), MINOR(tic_dev));

	return 0;

fail_mutex:
	mutex_unlock(&rw);
fail_add:
	cdev_del(tic_cdev);
fail_cdev:
	unregister_chrdev_region(tic_dev, 1);
fail_region:
	return ret;
}

static void tic_exit(void) {
	printk(KERN_INFO "Tic Tac Toe exit called\n");
	cdev_del(tic_cdev);
	unregister_chrdev_region(tic_dev, 1);
}

module_init(tic_init);
module_exit(tic_exit);
