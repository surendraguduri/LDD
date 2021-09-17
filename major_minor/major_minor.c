//#include <linux/kernel.h>
//#include <linux/module.h>
//#include <linux/init.h>
//#include <linux/fs.h>

//#include <linux/kdev_t.h>
//#include <linux/cdev.h>
//#include <linux/device.h>
//#include <linux/fcntl.h> /Helps fix O_ACCMODE/
//#include <linux/sched.h> /*Helps fix TASK_UNINTERRUPTIBLE */
//#include <linux/fs.h> /*Helps fix the struct intializer */

#include 	<linux/kernel.h>
#include	<linux/module.h>
#include	<linux/init.h>
#include 	<linux/slab.h>        /* kmalloc() */
#include	<linux/fs.h>
//#include	<linux/mm.h>
#include	<linux/cdev.h>
//#include	<linux/errno.h>
#include	<linux/types.h>
//#include	<linux/interrupt.h>
//#include	<linux/delay.h>
//#include    <linux/errno.h>
//#include    <linux/sched.h>
//#include    <linux/semaphore.h>
//#include    <linux/system.h>
#include	<linux/uaccess.h>/* */
//#include	<asm/arch/irqs.h>
//#include 	<linux/io.h>
//#include 	<linux/version.h>
//#include    <asm/hardware.h>

//#include <sys/types.h> 
#define MY_MAGIC 'a'
#define WR_VALUE _IOW(MY_MAGIC,'a',int32_t*) //(copy_from_user)
#define RD_VALUE _IOR(MY_MAGIC,'b',int32_t*) //(copy_to_user)
#define MY_IOCTL_MAX_CMD 2

dev_t dev =0;


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johny");
MODULE_DESCRIPTION(" major and minor  driver");
MODULE_VERSION("1.0.0");

static struct class *dev_class;
static struct cdev my_cdev;
uint8_t *kernel_buffer =NULL;
int32_t value = 0;
#define MEM_SIZE        1024           //Memory Size

static int      __init my_driver_init(void);
static void     __exit my_driver_exit(void);
static int      my_open(struct inode *inode, struct file *file);
static int      my_release(struct inode *inode, struct file *file);
static ssize_t  my_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  my_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = my_read,
    .write      = my_write,
    .open       = my_open,
    .unlocked_ioctl = my_ioctl,
    .release    = my_release,
};


static int my_open(struct inode *inode, struct file *file)
{
	
	if((kernel_buffer = kmalloc(MEM_SIZE,GFP_KERNEL))==0)
	{
		printk(KERN_INFO "Cannot allocate memory in kernel\n");
		return -1;
	}
	printk(KERN_INFO"Driver open  function called .....\n");
	return 0;	
}
static int my_release(struct inode *inode, struct file *file)
{
	kfree(kernel_buffer);
	printk(KERN_INFO"Driver release function called ....\n");
	return 0;
}
static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	int ret =0;
	//Copy the data from the kernel space to the user-space
    ret = copy_to_user(buf, kernel_buffer, MEM_SIZE);
	if(ret > 0)
	{
		printk(KERN_INFO"writing data to user-space failed\n");
	}
	printk(KERN_INFO"Driver read function called .....\n");
	return 0;
}
static ssize_t my_write(struct file *filp, const char __user *buf,size_t len,loff_t *off)
{
	int ret = 0;
	//Copy the data to kernel space from the user-space
	copy_from_user(kernel_buffer, buf, len);
	if(ret > 0)
	{
		printk(KERN_INFO"Copy the data to kernel space from the user-space\n");
	}
	printk(KERN_INFO"Driver write function called .... \n");
	return len;
} 

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	printk(KERN_INFO"Enter ioctl ");
			
	switch(cmd) {
		case WR_VALUE:
			printk(KERN_INFO"Enter write ");
			copy_from_user(&value ,(int32_t*) arg, sizeof(value));
			printk(KERN_INFO "Value = %d\n", value);
			break;
		case RD_VALUE:
			printk(KERN_INFO"Enter read ");
			copy_to_user((int32_t*) arg, &value, sizeof(value));
			 break;
	}
	return 0;
 }


static int __init my_driver_init(void)
{
	if((alloc_chrdev_region(&dev,0,2,"my_Char"))<0)// my_char is my driver name 
	{
		printk(KERN_INFO"cannot allocate major and minor number");
		return -1;
	}
	printk(KERN_INFO"Major =%d Minor = %d \n",MAJOR(dev),MINOR(dev));
	
	//creating cdev structure
	cdev_init(&my_cdev,&fops);
	
	//Adding charriver to system
	if((cdev_add(&my_cdev,dev,1)) <0)
	{
		printk(KERN_INFO"cannot add the device to the system \n");
		goto r_class;
	}

	if((dev_class = class_create(THIS_MODULE,"my_Char_class")) == NULL){
		printk(KERN_INFO"Cannot create the struct class for device");
		goto r_class;
	}
	if((device_create(dev_class,NULL,dev,1,"my_Char_driver"))==NULL) {
		printk(KERN_INFO" cannot create the device\n");
		goto r_device;
	}

	printk(KERN_INFO"character driver init sucess\n");

	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

static void __exit my_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev,1);
	printk(KERN_INFO "kernel driver removed  ... done \n");

}
module_init(my_driver_init);
module_exit(my_driver_exit);