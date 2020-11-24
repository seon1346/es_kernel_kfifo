#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

struct kfifo fifo;
struct kfifo new_fifo;
int N = 0;
int M = 0;
int read_flag = 0;
module_param(N, int, 0);
module_param(M, int, 0);
#define DEV_MAJOR_NUMBER 275
#define DEV_NAME "BufferedMem"



static ssize_t fifo_driver_write(struct file * file, const char*buf, size_t len, loff_t * ofs) { //len = 길이 +1('\0')포함
	int i = 0;
	char c;
	char tmp;
	printk(KERN_INFO " write!\n");


	for (i = 0; i < len - 1; i++) {
		if(kfifo_len(&fifo)>=N)
			kfifo_out(&fifo, &tmp,1);
		copy_from_user(&c, buf+i, 1);
		kfifo_in(&fifo, &c, 1);
	}
		
	return len;
}

static ssize_t fifo_driver_read(struct file * file, char * buf, size_t length, loff_t * ofs) {

	char *c;	
	int m = M;
	if (read_flag == 1) {
		read_flag = 0;
			return 0;
	}
	printk(KERN_INFO " read !\n");
	if (kfifo_len(&fifo) < M)
		m = kfifo_len(&fifo);
	c = kmalloc(m, GFP_KERNEL);

	kfifo_out(&fifo, c, m);
	copy_to_user(buf, c, m);
	read_flag = 1;
	
	kfree(c);
	return m;
}


static int fifo_driver_open(struct inode * inode, struct file * file) {
	printk(KERN_INFO "driver_open!\n");
	return 0;
}

static int fifo_driver_release(struct inode * inode, struct file * file) {
	printk(KERN_INFO "driver_release!\n");
	read_flag = 0;
	return 0;
}



static long fifo_driver_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	int x;
	int old_len = 0;
	int i = 0;
	char * tmp;
	char ctmp;
	x = _IOC_NR(cmd);
	switch (x) {
	case 1: //N변경 (기존것 보존.)
		copy_from_user(&N, (int *)arg, sizeof(int));

		printk(KERN_INFO "change N -> %d\n", N);
		old_len = kfifo_len(&fifo);
		tmp = kmalloc(old_len, GFP_KERNEL);
		kfifo_out(&fifo, tmp, old_len);

		kfifo_free(&fifo);
		kfifo_alloc(&fifo, N, GFP_KERNEL);
		for (i = 0; i < old_len; i++) {
			if (kfifo_len(&fifo) >= N) {
				kfifo_out(&fifo, &ctmp, 1);
				printk(KERN_INFO "Loss %c\n", ctmp);
			}
			kfifo_in(&fifo, tmp+i, 1);
		}
		kfree(tmp);
		printk(KERN_INFO "kfifo size:%d\n", N);
		printk(KERN_INFO "kfifo len:%d\n", kfifo_len(&fifo));
		printk(KERN_INFO "kfifo avail:%d\n", N - kfifo_len(&fifo));
		printk(KERN_INFO "kfifo read:%d\n", M);
		break;


	case 2: //M변경
		copy_from_user(&M, (int *)arg, sizeof(int));
		printk(KERN_INFO "chanege M -> %d\n", M);

		printk(KERN_INFO "kfifo size:%d\n", N);
		printk(KERN_INFO "kfifo len:%d\n", kfifo_len(&fifo));
		printk(KERN_INFO "kfifo avail:%d\n", N - kfifo_len(&fifo));
		printk(KERN_INFO "kfifo read:%d\n", M);

		break;
	default:
		
		break;
	}


	return 0;
}


static struct file_operations fifo_driver_fos= {
	.owner = THIS_MODULE,
	.write = fifo_driver_write,
	.read = fifo_driver_read,
	.release= fifo_driver_release,
	.open= fifo_driver_open,
	.unlocked_ioctl = fifo_driver_ioctl,
};


static int fifo_driver_init(void) {
	int ret;
	printk(KERN_INFO " Device driver registered!\n");

	ret=kfifo_alloc(&fifo, N, GFP_KERNEL);
	if (ret) {
		printk(KERN_ERR "error kfifo_alloc\n");
	}


	printk(KERN_INFO "kfifo size:%d\n", N);
	printk(KERN_INFO "kfifo len:%d\n", kfifo_len(&fifo));
	printk(KERN_INFO "kfifo avail:%d\n", N-kfifo_len(&fifo));
	printk(KERN_INFO "kfifo read:%d\n", M);
	register_chrdev(DEV_MAJOR_NUMBER, DEV_NAME, &fifo_driver_fos);
	return 0;
}

static void fifo_driver_exit(void) {
	printk(KERN_INFO " Device driver unregistered\n");
	unregister_chrdev(DEV_MAJOR_NUMBER, DEV_NAME);
	kfifo_free(&fifo);

}

module_init(fifo_driver_init);
module_exit(fifo_driver_exit);
MODULE_LICENSE("GPL");

