#include <linux/mm.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/backing-dev.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/uio.h>

#include <linux/uaccess.h>
#include<linux/module.h>

static int major;
static unsigned char hello_buf[100];
static struct  class *hello_class;


static int hello_open (struct inode *node, struct file *file)
{
	printk("%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
	return 0;
}

static ssize_t hello_read (struct file *file, char __user * buf, size_t size, loff_t *offset)
{
	unsigned long len = size > 100 ? 100 : size; 

	printk("%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);

	copy_to_user( buf, hello_buf, len);

	return 0;


}

static ssize_t hello_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int retval;
	unsigned long len = size > 100 ? 100 : size;

	printk("%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);

	retval = copy_from_user(hello_buf, buf, len); 

	if(retval < 0)
	{
		printk("kernel write failed!\r\n");
	}

	return len;



}

static int hello_release(struct inode *node, struct file *file)
{
		printk("%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
		return 0; 

}





/*1.创建fileoperation*/
static struct file_operations hello_drv = 
{
	.owner		= THIS_MODULE,
	.read		= hello_read,
	.write		= hello_write,
	.open		= hello_open,
	.release    = hello_release,
};


/*2.register_chrdev*/
static int hello_init(void)
{
	/*1.注册字符设备
	register_chrdev用于注册字符设备，三个参数，
	major 主设备号
	name 设备名称
	fops 结构体类型指针，设备操作函数集合变量
	*/
	major = register_chrdev(0, "hello_drv", &hello_drv);

	if(major < 0)
	{
		printk("register dev failed!\r\n");
		return -1;
	}
	// alloc_chrdev_region(&hello_drv,0,1,"hello_drv");

	/*2.class_create创建设备class类
	THIS_MODULE
	name 类名称
	*/
	hello_class = class_create(THIS_MODULE,"hello_class");
	if (IS_ERR(hello_class)) 
	{
		printk("failed to allocate class\n");
		return PTR_ERR(hello_class);
	}

	/* 3.创建设备device_create
		1.hello_class 创建的设备类
		2.parent 父设备，一般没有父设备也就是null
		3.设备号
		4.null
		5.设备名称
	*/
	device_create(hello_class, NULL, MKDEV(major,0), NULL, "hello"); 		/*/dev*/
	
	return 0;

}




/*3.entry_function*/
module_init(hello_init);





/*4.exit function*/

static void hello_exit(void)
{
	device_destroy(hello_class,MKDEV(major,0));

	class_destroy(hello_class);

	unregister_chrdev(major,"hello_drv");

	// unregister_chrdev_region(&hello_drv, 1);
	return 0;

}
module_exit(hello_exit);

MODULE_LICENSE("GPL");

MODULE_AUTHOR("111111");




