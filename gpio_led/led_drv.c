#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>


#if 1 
/*定义结构体*/
struct  gpio_desc
{
    int gpio;
    int irq;
    char *name;
    int key;
    struct timer_list key_timer;
};

/*定义设备结构体数组*/
static struct gpio_desc gpios[2] = {
        {20, 0 , "led0",},

};




/*主设备号*/
static int major = 0;
static struct class *gpio_class;




/*驱动里读函数*/
static ssize_t gpio_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    char _tempbuff[2];
    int err;

    if(2 != size)
    {
        return -EINVAL;
    }

    err = copy_from_user(_tempbuff, buf, 1);



    _tempbuff[1] = gpio_get_value(gpios[_tempbuff[0]].gpio);

    err = copy_to_user(buf, _tempbuff, 2);

    return 2;








}

/*驱动里写函数*/
static ssize_t gpio_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    unsigned char _ker_buff[2];
    int err;

    if(2 != size)
    {
        return -EINVAL;
    }

    err = copy_from_user(_ker_buff, buf, size);

    if(_ker_buff[0] >= sizeof(gpios)/sizeof(gpios[0]))
    {
        return -EINVAL;
    }

    gpio_set_value(gpios[_ker_buff[0]].gpio, _ker_buff[1]);
    return 2;



}



/*1.构造file_operation*/
static const struct file_operations gpio_led_fops  ={
	.owner = THIS_MODULE,
	
	.read = gpio_drv_read,
    .write = gpio_drv_write,
};



/*2.初始化*/
static int __init gpio_drv_init(void)
{
    int err;
    int i;
    int count = sizeof(gpios)/sizeof(gpios[0]);
   
    printk("%s   %s  line %d\n",__FILE__, __FUNCTION__, __LINE__);

    for(i = 0; i < count; i++) 
    {
        err = gpio_request(gpios[1].gpio, gpios[1].name);

        if(err < 0)
        {
            printk("can not request gpio %s %d\n",gpios[1].name, gpios[1].gpio);
            return -ENAVAIL;
        }
        gpio_direction_output(gpios[i].gpio, 1);//set gpio direction
    }
    
    /*register file operation*/
    major = register_chrdev(0, "wgw_led_gpio", &gpio_led_fops);

    /*create class*/
    gpio_class = class_create(THIS_MODULE, "gpio_led_class");

    if(IS_ERR(gpio_class))
    {
        printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
        unregister_chrdev(major, "wgw_led_gpio");    
        return PTR_ERR(gpio_class);
    }

    device_create(gpio_class, NULL, MKDEV(major, 0), NULL, "wgw_led_gpio");

    return err;




}

/*3.退出函数*/
static void __exit gpio_drv_exit(void)
{
    int i;
    int count = sizeof(gpios)/sizeof(gpios[0]);

    printk("%s %s line %d\n",__FILE__,__FUNCTION__,__LINE__);

    device_destroy(gpio_class, MKDEV(major, 0));
    class_destroy(gpio_class);
    unregister_chrdev(major, "wgw_led_gpio"); 

    for(i = 0; i < count; i++) 
    {
        gpio_free(gpios[i].gpio);
    }    
}


module_init(gpio_drv_init);
module_exit(gpio_drv_exit);

MODULE_LICENSE("GPL");
#endif


