#include "asm-generic/errno-base.h"
#include "asm/cacheflush.h"
#include "linux/cdev.h"
#include "linux/fs.h"
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
#include <linux/shmem_fs.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/uio.h>

#include <linux/uaccess.h>


#define hello_dev_cnt  1
#define hello_dev_name "new_hello_cdev"
/****************************************************************/
/******************使用cdev结构体表示一个字符设备********************/ 
struct hello_dev{
    dev_t  devid;           //设备号
    struct cdev hello_cdev; //创建一个cdev
    struct class *hello_class;//创建一个类
    struct device *device;  //设备
    int major;              //主设备号
    int minor;              //次设备号   
};

struct hello_dev hello_dev;
static unsigned char hello_buff[100];



/*操作函数*/
/*open*/
static int hello_open (struct inode *node, struct file *filp)
{
    filp->private_data = &hello_dev;        /*设置私有数据*/
    return 0;
}

/*read*/
static ssize_t hello_read (struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    int retval;
    unsigned long len = size > 100 ? 100 : size;

    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

    retval = copy_to_user(buf, hello_buff, len);

    if(retval < 0) 
    {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}
    return len;



}

/*write*/
static ssize_t hello_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
    int retval;

    unsigned long len = size > 100 ? 100 : size;
    
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    
    retval = copy_from_user(hello_buff, buf, len);
    
    if(retval < 0) 
    {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

    return len;

}

/*release*/
static int hello_release (struct inode *node, struct file *filp)
{
    return 0;
}




/*1.创建file_operation*/
static const struct file_operations hello_drv = {
    .owner      = THIS_MODULE,
	.read		= hello_read,
	.write		= hello_write,
	.open		= hello_open,
    .release    = hello_release,
};


/*2.初始化入口*/
static int hello_init(void)
{
    /*注册字符设备驱动*/

    /*1.创建设备号*/
    if(hello_dev.major) //如果定义了设备号
    {
        hello_dev.devid = MKDEV(hello_dev.major, 0); //如果给了主次设备号，就用下面这个函数注册设备
        register_chrdev_region(hello_dev.devid, hello_dev_cnt, hello_dev_name);
    }
    else                        //没有定义设备号
    {
        alloc_chrdev_region(&hello_dev.devid, 0, hello_dev_cnt, hello_dev_name);//如果没给设备号，就用这个函数注册设备
        hello_dev.major = MAJOR(hello_dev.devid);
        hello_dev.minor = MINOR(hello_dev.devid);
    }

    printk("hello_dev major = %d,minjor = %d\r\n",hello_dev.major,hello_dev.minor);

    /*2.初始化cdev*/
    hello_dev.hello_cdev.owner = THIS_MODULE;
    cdev_init(&hello_dev.hello_cdev, &hello_drv); //cdev和drv

    /*3.增加一个cdev*/
    cdev_add(&hello_dev.hello_cdev, hello_dev.devid, hello_dev_cnt);

    /*4.创建类*/
    hello_dev.hello_class = class_create(THIS_MODULE, hello_dev_name);
    if(IS_ERR(hello_dev.hello_class))
    {
        return PTR_ERR(hello_dev.hello_class);
    }

    /*5.创建设备*/
    hello_dev.device = device_create(hello_dev.hello_class, NULL, hello_dev.devid, NULL, hello_dev_name);

    if(IS_ERR(hello_dev.device))
    {
        return PTR_ERR(hello_dev.device);
    }

    return 0;

}


/*3.退出*/
static void hello_exit(void)
{
    /*注销字符设备*/
    cdev_del(&hello_dev.hello_cdev);
    unregister_chrdev_region(hello_dev.devid, hello_dev_cnt);//注销字符设备，不管前面是怎么注册的

    device_destroy(hello_dev.hello_class, hello_dev.devid);//销毁设备

    class_destroy(hello_dev.hello_class); //销毁类
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");

