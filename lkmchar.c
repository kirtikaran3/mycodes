#include<linux/init.h> 
#include<linux/module.h> 
#include<linux/kernel.h> 
#include<linux/fs.h> 
#include<linux/cdev.h> 
#include<linux/semaphore.h> 
#include<linux/uaccess.h> 

struct fake_device{

	char data[100]; 
	struct semaphore sem;
	
}virtualdevice;  

int ret;

struct cdev *mcdev; 
int major_number; 
dev_t dev_num; 
#define DeviceName "FakeCharDriver" 

int device_open(struct inode *inode,struct file *filp){

	if(down_interruptible(&virtualdevice.sem)!=0){

		printk(KERN_ALERT "Couldnot open\n"); 
		return -1;
	} 
	printk(KERN_INFO"Open Success");
	return 0;

} 


ssize_t device_read(struct file* filp,char* bufStoredata,size_t bufcount,loff_t* curOffset){

	printk(KERN_INFO"Reading from device"); 
	ret=copy_to_user(bufStoredata,virtualdevice.data,bufcount); 
	return ret;
} 

ssize_t device_write(struct file* filp,const char* bufSourceData,size_t bufCount,loff_t* curOffset){ 
	
	printk(KERN_INFO "Writing to driver"); 
	ret=copy_from_user(virtualdevice.data,bufSourceData,bufCount); 
return ret;
}

int device_close(struct inode *inode,struct file *filp){

	up(&virtualdevice.sem); 
	printk(KERN_INFO" close th driver"); 
	return 0;
}



struct file_operations fops ={

	.owner=THIS_MODULE, 
	.open=device_open, 
	.release=device_close, 
	.write=device_write, 
	.read=device_read,

}; 

static int driver_entry(void){


	ret=alloc_chrdev_region(&dev_num,0,1,DeviceName); 
	
		if(ret<0){

			printk(KERN_ALERT"Failed to allocate\n");
			return ret;
		}

	major_number=MAJOR(dev_num); 
	printk(KERN_INFO"Device major no is %d with name %s",major_number,DeviceName); 
	mcdev=cdev_alloc(); 
	mcdev->ops=&fops; 
	mcdev->owner=THIS_MODULE;
	ret=cdev_add(mcdev,dev_num,1); 

		if(ret<0){

			printk(KERN_ALERT"undable to add"); 
			return -1;
		} 

	sema_init(&virtualdevice.sem,1); 
return 0;
}  
static void driver_exit(void){

	cdev_del(mcdev); 
	unregister_chrdev_region(dev_num,1); 
	printk(KERN_ALERT "Unreg the moduel"); 
	

} 
module_init(driver_entry); 
module_exit(driver_exit);






















