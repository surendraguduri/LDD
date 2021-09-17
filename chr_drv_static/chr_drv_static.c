#include <linux/module.h>
#include<linux/moduleparam.h>

#include <linux/fs.h>

#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
 

MODULE_LICENSE("GPL");
MODULE_LICENSE("GPL v2");
MODULE_LICENSE("Dual BSD/GPL");

MODULE_AUTHOR("roshini");
MODULE_DESCRIPTION("A sample driver");
MODULE_VERSION("1:0.3");

#define MAJORNO 300 // unique id 
#define MINORNO  0 
#define CHAR_DEV_NAME "my_cdev"

#define MAX_LENGTH 4000
#define SUCCESS 0

struct cdev *my_cdev;
dev_t mydev;
int count=1,inuse=0;

static int char_dev_open(struct inode *inode,
			    struct file  *file)
{	
	printk(KERN_INFO "Open operation invoked \n");
	return SUCCESS;
		
}

static int char_dev_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "close operation invoked \n");
	return SUCCESS;
}

static ssize_t char_dev_write(struct file *file, const char *buf, size_t lbuf, loff_t *ppos)
{
       printk(KERN_INFO "write operation invoked \n");

       return 0;

}
static struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	.write = char_dev_write,
	.open = char_dev_open,
	.release = char_dev_release
};

static int __init hello_world_init(void) /* Constructor */
{

				
	printk(KERN_INFO"hello world  started ");
	int ret =0;
	mydev = MKDEV(MAJORNO,MINORNO); // mydev => 32 bit  ; 20 major no. 12 minor no. 

	register_chrdev_region(mydev,count,CHAR_DEV_NAME); 

	my_cdev= cdev_alloc();  // allow allocate cdev instance, 
	cdev_init(my_cdev,&char_dev_fops);  

	ret=cdev_add(my_cdev,mydev,count);// register with VFS layer , count = how many minor no. in use .
	
	if( ret < 0 ) {
		printk("Error registering device driver\n");
		return ret;
	}
	printk(KERN_INFO"\nDevice Registered %s\n",CHAR_DEV_NAME); 
 	
	return ret;
}

module_init(hello_world_init);


static void __exit hello_world_exit(void)
{
	cdev_del(my_cdev);
	unregister_chrdev_region(mydev,count);
 	printk(KERN_INFO"hello world  exit ");
}
module_exit(hello_world_exit)