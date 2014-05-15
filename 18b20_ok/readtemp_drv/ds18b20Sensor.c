#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <mach/regs-gpio.h>
#include <linux/device.h>
#include <mach/hardware.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/errno.h>

#include "s3c6410_gpio.h"

//#define DEBUG


#define DEVICE_NAME "ds18b20_sensor"
#define DQ         0
#define CFG_IN     0
#define CFG_OUT    1


int ds18b20_major = 0;
int ds18b20_minor = 0;
int ds18b20_nr_devs = 1;


static struct ds18b20_device {
    struct cdev cdev;
};
struct ds18b20_device ds18b20_dev;

static struct class *ds18b20_class;


static int ds18b20_open(struct inode *inode, struct file *filp);
static int ds18b20_init(void);
static void write_byte(unsigned char data);
static unsigned char read_byte(void);
static ssize_t ds18b20_read(struct file *filp, char __user *buf,
                            size_t count, loff_t *f_pos);
void ds18b20_setup_cdev(struct ds18b20_device *dev, int index);

unsigned char mybuf[8]; 


void msdelay(unsigned int i)    
{
    for(i=i;i>0;i--)
        udelay(1000);
}


static int ds18b20_open(struct inode *inode, struct file *filp)
{
    int flag = 0;
    /*struct ds18b20_device *dev;
    dev = container_of(inode->i_cdev, struct ds18b20_device, cdev);
    filp->private_data = dev;*/

    flag = ds18b20_init();
    if(flag & 0x01)
    {
#ifdef DEBUG
        printk(KERN_WARNING "open ds18b20_sensor failed\n");
#endif
	return -1;
    }
#ifdef DEBUG
    printk(KERN_NOTICE "open ds18b20_sensor successful\n");
#endif
    return 0;
}


static int ds18b20_init(void)
{
    int retval = 0;

    s3c6410_gpio_cfgpin(DQ, CFG_OUT);
    s3c6410_gpio_pullup(DQ, 0);

    s3c6410_gpio_setpin(DQ, 1);
    udelay(2);
    s3c6410_gpio_setpin(DQ, 0);        
    udelay(500);                      

    s3c6410_gpio_setpin(DQ, 1);       
    udelay(60);

    s3c6410_gpio_cfgpin(DQ, CFG_IN);
    retval = s3c6410_gpio_getpin(DQ);

    udelay(500);
    s3c6410_gpio_cfgpin(DQ, CFG_OUT);
    s3c6410_gpio_pullup(DQ, 0);
    s3c6410_gpio_setpin(DQ, 1);       
    
    return retval;
}


static void write_byte(unsigned char data)
{
    int i = 0;

    s3c6410_gpio_cfgpin(DQ, CFG_OUT);
    s3c6410_gpio_pullup(DQ, 1);

    for (i = 0; i < 8; i ++)
    {
        s3c6410_gpio_setpin(DQ, 1);
        udelay(2);
        s3c6410_gpio_setpin(DQ, 0);
        s3c6410_gpio_setpin(DQ, data & 0x01);
        udelay(60);
	data >>= 1;
    }
    s3c6410_gpio_setpin(DQ, 1);        
}


static unsigned char read_byte(void)
{
    int i;
    unsigned char data = 0;

    for (i = 0; i < 8; i++)
    {
        s3c6410_gpio_cfgpin(DQ, CFG_OUT);
        s3c6410_gpio_pullup(DQ, 0);
        s3c6410_gpio_setpin(DQ, 1);
        udelay(2);
        s3c6410_gpio_setpin(DQ, 0);
        udelay(2);
	s3c6410_gpio_setpin(DQ, 1);
        udelay(8);
        data >>= 1;
	s3c6410_gpio_cfgpin(DQ, CFG_IN);
	if (s3c6410_gpio_getpin(DQ))
	    data |= 0x80;
	udelay(50);
    }
    s3c6410_gpio_cfgpin(DQ, CFG_OUT);
    s3c6410_gpio_pullup(DQ, 0);
    s3c6410_gpio_setpin(DQ, 1);       
    return data;
}

static ssize_t ds18b20_read(struct file *filp, char __user *pData,
                            size_t count, loff_t *f_pos)
{
    int flag;
    unsigned long err;
    unsigned char i;
    unsigned char result[2] = {0x00, 0x00};
    flag = ds18b20_init();
    if (flag)
    {
#ifdef DEBUG
        printk(KERN_WARNING "ds18b20 init failed\n");
#endif
        return -1;
    }

    flag = ds18b20_init();
    if (flag)
        return -1;

    write_byte(0xcc);
    write_byte(0x44);

    mdelay(50);

    flag = ds18b20_init();
    if (flag)
        return -1;

    write_byte(0x55);  //match rom
	#ifdef DEBUG
    printk("match rom:");
	#endif
    for(i=0;i<8;i++)
    {
		write_byte(mybuf[i]);
		#ifdef DEBUG
		printk("0x%02x ",mybuf[i]);
		#endif
    }
	#ifdef DEBUG
    printk("\n");
	#endif


    write_byte(0xbe);
	
    result[0] = read_byte();    
    result[1] = read_byte(); 
    
    err = copy_to_user(pData, &result, sizeof(result));
    return err ? -EFAULT : min(sizeof(result),count);
}

ssize_t ds18b20_write(struct file *pFile,const char  __user *pData, size_t count, loff_t *off )
{
    unsigned char ret;
    unsigned char i;
    struct ds18b20_device *dev;
    dev=pFile->private_data;
	#ifdef DEBUG
    	printk(KERN_ALERT "kernel write ID:");
	#endif
    ret=copy_from_user(mybuf,pData, 8); 

	#ifdef DEBUG
    for(i=0;i<8;i++)
    {
		printk("0x%02x ",mybuf[i]);
    }
    printk("\n");
	#endif
    if(ret>0)
    {
        printk("copy data failed\n");
        return -1;
    }
    return 0;
}


static struct file_operations ds18b20_dev_fops = {
    .owner = THIS_MODULE,
    .open = ds18b20_open,
    .read = ds18b20_read,
    .write = ds18b20_write,
};


void ds18b20_setup_cdev(struct ds18b20_device *dev, int index)
{
    int err, devno = MKDEV(ds18b20_major, ds18b20_minor + index);

    cdev_init(&dev->cdev, &ds18b20_dev_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&(dev->cdev), devno, 1);
    if (err)
    {
#ifdef DEBUG
        printk(KERN_NOTICE "ERROR %d add ds18b20\n", err);
#endif
    }
}


static int __init ds18b20_dev_init(void)
{
    ds18b20_major = register_chrdev(ds18b20_major, DEVICE_NAME, &ds18b20_dev_fops);
    if (ds18b20_major<0)
    {
	printk(DEVICE_NAME " Can't register major number!\n");
	return -EIO;
    }

    ds18b20_class = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(ds18b20_class, NULL, MKDEV(ds18b20_major, ds18b20_minor), NULL, DEVICE_NAME);
#ifdef DEBUG
	printk(KERN_WARNING "register ds18b20_sensor successful!\n");
#endif
    return 0;
}


static void __exit ds18b20_dev_exit(void)
{
    device_destroy(ds18b20_class, MKDEV(ds18b20_major,ds18b20_minor));
    class_unregister(ds18b20_class);
    class_destroy(ds18b20_class);
    unregister_chrdev(ds18b20_major, DEVICE_NAME);
#ifdef DEBUG
	printk(KERN_WARNING "Exit ds18b20 driver!\n");
#endif
}

module_init(ds18b20_dev_init);
module_exit(ds18b20_dev_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Yang Deyao & Chen Lei");

