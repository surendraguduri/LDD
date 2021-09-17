#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/kdev_t.h>

#define MAJORNO 240
#define MINORNO  1
#define CHAR_DEV_NAME "my_cdev"
#define MAX_LENGTH 4000
#define SUCCESS 0

static char char_device_buf[MAX_LENGTH];
static struct class *dev_class;
struct cdev *my_cdev;
dev_t mydev;
int count=1,inuse=0;


static int char_dev_open(struct inode *inode,
			    struct file  *file)
{
	if(inuse)
	{
		printk(KERN_INFO "\nDevice busy %s\n",CHAR_DEV_NAME);
		return -EBUSY;
	}	
	inuse=1;
	printk(KERN_INFO "Open 1 operation invoked \n");
	return SUCCESS;
		
}

static int char_dev_release(struct inode *inode, struct file *file)
{
	inuse=0;
	printk(KERN_INFO "close 1 operation invoked \n");
	return SUCCESS;
}

static ssize_t char_dev_write(struct file *file, const char *buf, size_t lbuf, loff_t *ppos)
{
       int nbytes = lbuf - raw_copy_from_user (char_device_buf + *ppos, buf, lbuf);
       *ppos += nbytes;
       printk (KERN_INFO "\n device 1 Rec'vd data from app %s , nbytes=%d\n",char_device_buf,nbytes);

       return nbytes;

}

static struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	.write = char_dev_write,
	.open = char_dev_open,
	.release = char_dev_release
};

static int __init char_dev_init(void)
{
	int ret;
	mydev = MKDEV(MAJORNO,MINORNO);
	register_chrdev_region(mydev,count,CHAR_DEV_NAME); 

	my_cdev= cdev_alloc();  // allow allocate cdev instance, 
	cdev_init(my_cdev,&char_dev_fops);  

	ret=cdev_add(my_cdev,mydev,1);// register with VFS layer , count = how many minor no. in use .
	
	if( ret < 0 ) {
		printk("Error registering device driver\n");
		return ret;
	}
	
	if((dev_class = class_create(THIS_MODULE,"my_Char_class1")) == NULL){
		printk(KERN_INFO"Cannot create the struct class for device");
		goto r_class;
	}
	if((device_create(dev_class,NULL,mydev,1,"my_Char_driver1"))==NULL) {
		printk(KERN_INFO" cannot create the device\n");
		goto r_device;
	}
	printk(KERN_INFO"\nDevice Registered %s\n",CHAR_DEV_NAME); 
	
	memset(char_device_buf,'\0',MAX_LENGTH);
	return 0;
r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(mydev,1);
	return -1;
}

static void __exit char_dev_exit(void)
{
	 device_destroy(dev_class,mydev);
	 class_destroy(dev_class);
	 cdev_del(my_cdev);
	 unregister_chrdev_region(mydev,1);
	 printk(KERN_INFO "\n Device unregistered \n");
}

module_init(char_dev_init);
module_exit(char_dev_exit);

MODULE_AUTHOR("Johny");
MODULE_DESCRIPTION("Static Character Device Driver");
MODULE_LICENSE("GPL");
/* End of code */