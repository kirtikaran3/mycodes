#include "header.h"

/*

loff_t scull_lseek(struct file *filp,loff_t offset, int origin)
{
	int newpos;

	#ifndef DEBUG
	printk(KERN_INFO"Begin:%s",__func__);
	#endif
	
	
	switch(origin)
	{
		case 0:
		{
			if(offset<0)
			{
				printk(KERN_INFO"Error...in lseek\n");
				goto OUT;
			}	
			
			newpos=offset;

		printk(KERN_INFO" offset = %d\n",newpos);
			break;
		}
		case 1:
		{

			break;
		}
		case 2:
		{
			if(offset>0)
			{
				printk(KERN_INFO"Error...in lseek\n");
				goto OUT;
			}	




			break;
		}
		default:
			goto OUT;


	}
	filp->f_pos=newpos;
	#ifndef DEBUG
	printk(KERN_INFO"End:%s",__func__);
	#endif
	return 0;
OUT :
	#ifndef DEBUG
	printk(KERN_INFO"End with -1 return :%s",__func__);
	#endif
	return -1;
}

*/

int scull_quantum(struct ScullQset *fscullqset,int fqset)
{
	int lv;
	
	#ifndef DEBUG
	printk(KERN_INFO"Begin:%s",__func__);
	#endif

	if(!fscullqset)
	{
		#ifndef DEBUG
		printk(KERN_INFO"Invalid address");
		#endif
		goto OUT;
	}

	fscullqset->data=kmalloc(sizeof(char *)*fqset,GFP_KERNEL);
	memset(fscullqset->data,0,sizeof(char *)*fqset);

	for(lv=0;lv<fqset;lv++)
	{
		
		fscullqset->data[lv]=kmalloc(sizeof(char)*QUANTUM,GFP_KERNEL);
//		memset(fscullqset->data[lv],'\0',sizeof(char *)*QUANTUM);

		if(fscullqset->data[lv])
		{
			#ifndef DEBUG
			printk(KERN_ALERT "Initialized data[%d] with %p\n",lv,fscullqset->data[lv]);
			#endif
		}
		
		else
		{
			#ifndef DEBUG
			printk(KERN_ALERT "Initialized failed");
			#endif

			goto OUT;
		}

	}		


	
	#ifndef DEBUG
	printk(KERN_INFO"End:%s",__func__);
	#endif
	return 0;
OUT :
	#ifndef DEBUG
	printk(KERN_INFO"End with -1 return :%s",__func__);
	#endif
	return -1;
}




struct ScullQset *scull_follow(struct ScullDev *fsculldev)
{
	struct ScullQset *lscullqset;
	#ifndef DEBUG
	printk(KERN_INFO"Begin:%s",__func__);
	#endif

		
	lscullqset=fsculldev->scullqset;

	if(!lscullqset)
	{
		lscullqset=kmalloc(sizeof(struct ScullQset), GFP_KERNEL);
//		memset(lscullqset,0,sizeof(struct ScullQset));
		if(lscullqset)
		{
			#ifndef DEBUG
			printk(KERN_ALERT"initialize sculldev memory\n");
			#endif
		}
		else
		{
			#ifndef DEBUG
			printk(KERN_ALERT"Failed to intialize sculldev memory\n");
			#endif
		}	
		goto OUT;
	}
	fsculldev->qset=0;
	while(lscullqset->next)
	{
		lscullqset=lscullqset->next;
		fsculldev->qset++;
	}

	#ifndef DEBUG
	printk(KERN_INFO"End:%s",__func__);
	#endif

	return lscullqset;

OUT: 	
	#ifndef DEBUG
	printk(KERN_INFO"FOLLOW :(:%s",__func__);
	#endif
	return lscullqset;
}


int scull_read(struct file *filp,char __user *buf,size_t count ,loff_t *f_pos)
{
//	int retval;
	
	int qset=QSET;				
//	struct ScullQset *lscullqset;

	struct ScullDev *lsculldev=filp->private_data;
	
	int nq,lv,rq,i,item;
	int quantum=lsculldev->quantum=QUANTUM;
	int cw=0,lcount;

	#ifndef DEBUG
	printk(KERN_INFO"Begin:%s",__func__);
	#endif
	
	if(count%quantum==0)
		nq=count/quantum;
	else
		nq=count/quantum+1;	


	if(nq%qset==0)
		item=nq/qset;
	else
		item=nq/qset+1;

	printk(KERN_INFO" number of quantum allocated are %d and item are %d\n",nq,item);

	for(i=0;i<item;i++)
	{
		if((nq-quantum)>0)
		{
			rq=quantum;
			nq=nq-quantum;
		}
		else
			rq=nq;
		
		printk(KERN_INFO "  The rq= %d and nq=%d\n\n",rq,nq);		
		for(lv=0;lv<rq;lv++)
		{	
		if((count-cw)>=quantum)
				lcount=quantum;
		else if((count-cw)<quantum)
				lcount=count-cw;
	
			if(lv=0)
			{
				lcount= quantum-(*f_pos);
				
				if(copy_to_user(buf+*f_pos,lsculldev->scullqset->data[lv],lcount))
				{
		
					#ifndef DEBUG
					printk(KERN_INFO"copy_from_user failed");
					#endif
					goto OUT;
				}
				printk(KERN_INFO" read = %s \n\n",buf+*f_pos);	


			}

			else
			{
				if((count-cw)>=quantum)
					lcount=quantum;
				else if((count-cw)<quantum)
					lcount=count-cw;

				if(copy_to_user(buf+*f_pos,lsculldev->scullqset->data[lv],lcount))
				{
		
					#ifndef DEBUG
					printk(KERN_INFO"copy_from_user failed");
					#endif
					goto OUT;
				}
				printk(KERN_INFO" read = %s \n\n",buf+*f_pos);	
			}
			*f_pos+=lcount;
			cw +=lcount;
	
		}

		lsculldev->scullqset=lsculldev->scullqset->next;
	}

	#ifndef DEBUG
	printk(KERN_INFO"End:%s",__func__);
	#endif
	return count;
OUT:  
	#ifndef DEBUG
	printk(KERN_INFO"ERROR:%s",__func__);
	#endif
	return -1;
}

int scull_write(struct file *filp,const char __user *buf,size_t count ,loff_t *f_pos)
{

	struct ScullQset *lscullqset;
	struct ScullDev *lsculldev=filp->private_data;
	int qset=lsculldev->qset=QSET;				
	int quantum=lsculldev->quantum=QUANTUM;
	int size=lsculldev->size=SIZE;
	int lqset,retval,lv;
	int nq,item,i,rq,sq;
	int cw=0,lcount;
	#ifndef DEBUG
	printk(KERN_INFO"Begin:%s",__func__);
	#endif

	//Fetching the device from private data which was saved during open system call, as the private data store all the attributes related to that file(device)


	//Initializing qset,quantum &size to sculldev as well as the respective elements of sculldev.

	
	#ifndef DEBUG
	printk(KERN_INFO"qset size= %d Quantum=%d size=%d count=%d\n",qset,quantum,size,count);
	printk("\n");
	#endif

	//If data writen ( i.e. store in count) is more than 64byts (quantum*qset) than dont perform write operation,as there is insufficient memory	
	
	if(size<count)
	{
		#ifndef DEBUG
		printk(KERN_INFO"Not enough of memory\n");
		#endif
		goto OUT;
	}

	//calculated no of qset required to write the data

	lqset=count/qset;
	if(count%qset)					// as if the size of data is not a multiple of quantum size
		lqset++;

	#ifndef DEBUG
	printk(KERN_INFO"lqset=%d\n",lqset);
	#endif

	//scull_follow will allocate the memory for the nextavailable scullqset

	lsculldev->scullqset=lscullqset=scull_follow(lsculldev);
	
	if (lscullqset == NULL)
		goto OUT;
	
	// scull_quantum will allocate the memory for required number of quantum.
		
	if(lqset>qset)
		lqset=qset;

	retval=scull_quantum(lscullqset,lqset);		

	if(!retval)
	{		
		#ifndef DEBUG
		printk(KERN_ALERT"Memory allocate :)\n");
		#endif
	}

	else
	{
		#ifndef DEBUG
		printk(KERN_ALERT"failed to allocate memory\n");
		#endif
		goto OUT;
	}

	if(count%quantum==0)
		nq=count/quantum;
	else
		nq=count/quantum+1;	


	if(nq%qset==0)
		item=nq/qset;
	else
		item=nq/qset+1;

	printk(KERN_INFO" number of quantum allocated are %d and item are %d\n",nq,item);


	for(i=0;i<item;i++)
	{
		if((nq-quantum)>0)
		{
			rq=quantum;
			nq=nq-quantum;
		}
		else
			rq=nq;
		
		printk(KERN_INFO "  The rq= %d and nq=%d\n\n",rq,nq);		
	
		for(lv=0;lv<rq;lv++)
		{
			if((count-cw)>=quantum)
				lcount=quantum;
			else if((count-cw)<quantum)
				lcount=count-cw;
	
			lscullqset->data[lv]=kmalloc(lcount,GFP_KERNEL);	
		
//		memset(lscullqset->data[lv],'\0',lcount);
			printk(KERN_INFO" memory allocated number %d is  %p\n",lv,lscullqset->data[lv]);

			if(copy_from_user(lscullqset->data[lv],buf+*f_pos,lcount))
			{
		
				#ifndef DEBUG
				printk(KERN_INFO"copy_from_user failed");
				#endif
				goto OUT;
			}
	

			#ifndef DEBUG
			printk(KERN_INFO"write %d no of bytes\n",lcount);
			printk(KERN_INFO"copy_from_user is %s\n",lscullqset->data[lv]);
			#endif

			*f_pos+=lcount;
			cw +=lcount;
	
		}
	
		if(nq>qset)
			sq=qset;
		else 
			sq=nq;

		lscullqset=lscullqset->next=kmalloc(sizeof(struct ScullQset),GFP_KERNEL);

		retval=scull_quantum(lscullqset,sq);		

		if(!retval)
		{		
			#ifndef DEBUG
			printk(KERN_ALERT"Memory allocate :)\n");
			#endif
		}

		else
		{
			#ifndef DEBUG
			printk(KERN_ALERT"failed to allocate memory\n");
			#endif
			goto OUT;
		}
	}

	#ifndef DEBUG
	printk(KERN_INFO"End:%s",__func__);
	#endif
	return count;
OUT :
	#ifndef DEBUG
	printk(KERN_INFO"End with -1 return :%s",__func__);
	#endif
	return -1;
}


int scull_setup_cdev(struct ScullDev *lsculldev ,int index)
{
//intiailzing the device through cdev_init	
	int retval;
	dev_t devno;
	
	devno=MKDEV(major,minor+index);
	
	cdev_init(&lsculldev->cdev,&fops);
	
	lsculldev->cdev.owner=THIS_MODULE;
	lsculldev->cdev.ops=&fops;
//Adding this device 

	retval=cdev_add(&lsculldev->cdev,devno,NO_OF_DEVICES);

	if(retval)
	{
		#ifndef DEBUG
		printk(KERN_ALERT"Failed to add the device\n");
		#endif
		goto OUT;
	}
	else
	{
		#ifndef DEBUG
		printk(KERN_ALERT"added the device with major No %d minor no %d\n",MAJOR(devno),MINOR(devno));
		#endif
	}

	return 0;

OUT: return -1;	
}

int scull_open(struct inode *inode,struct file *filp)
{
	
	int retval;

	printk(KERN_INFO"Begin:%s",__func__);
	
//	struct ScullDev *op_dev =NULL;
	
	op_dev=container_of(inode->i_cdev,struct ScullDev,cdev);
	
	if(op_dev)
	{		
		#ifndef DEBUG
		printk(KERN_ALERT"container of working fine\n");
		#endif
	}

	else
	{
		#ifndef DEBUG
		printk(KERN_ALERT"container of failed\n");
		#endif
		goto OUT;
	}
	filp->private_data=op_dev;

	if((filp->f_flags & O_ACCMODE) ==O_WRONLY)
			retval = scull_trim(op_dev);
	
	if(retval)
	{
		#ifndef DEBUG
		printk(KERN_ALERT"Failed to trim the scull\n");
		#endif
		goto OUT;
	}
	else
	{
		#ifndef DEBUG
		printk(KERN_ALERT"scull trimmed to 0 length\n");
		#endif
	}
	
	#ifndef DEBUG
	printk(KERN_INFO"End:%s:",__func__);
	#endif
	return 0;
OUT: return -1;
}

int scull_trim(struct ScullDev *lsculldev)
{
	struct ScullQset *dptr;
	struct ScullQset *next;
	int qset=lsculldev->qset;
	int i;
	
	printk(KERN_INFO"%d\n\n\n",lsculldev->qset);	

	for(dptr=lsculldev->scullqset;dptr;dptr=next)
	{
	//dptr initialized with the first array of scullqset the loop itrates
	//as dptr reaches NULL,after each iteration dptr will point to next scullqset array

		if(dptr->data) // scullqset is NULL then no need to free the memory		
		{
			i=0;
			while(dptr->data[i] && i<8)
			{
				kfree(dptr->data[i]); //free each array of data
				i++;
			}
			kfree(dptr->data);
			dptr->data=NULL;//since all the array of data is free hence scullqset->data can also be freed.
	
			next=dptr->next;		
			kfree(dptr);
		}
	}
		

	return 0;
}
int scull_release(struct inode *inode,struct file *filp)
{
		
	#ifndef DEBUG
	printk(KERN_INFO"Begin:%s",__func__);
	#endif

	#ifndef DEBUG
	printk(KERN_INFO"End:%s:",__func__);
	#endif
	return 0;
}


//////			The Initialization function		///////////////////


static int hello_init(void)
{
	

	int retval,i;
	#ifndef DEBUG
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	
	//Re-registration of the device, as after alloc-chrdev-region there will be some non zero major number, acording to which we have to assign diffrent minor numbers.

	if(major)
	{
		dev=MKDEV(major,minor+1);
		retval=register_chrdev_region(dev,NO_OF_DEVICES,"MYDEVICE");
		if(retval)
		{
			#ifndef DEBUG
			printk(KERN_ALERT"Re-registred the device with Major=%d Minor=%d\n",MAJOR(dev),MINOR(dev));
			#endif
		}	
		
		else
		{
	
			#ifndef DEBUG
			printk(KERN_ALERT"Registration unsuccesful\n");
			#endif
			goto OUT;
		}
	}
		//Alloc_chrdev_region used for registration when the major no is 0 which means only first time
	else
	{

		retval=alloc_chrdev_region(&dev,major,NO_OF_DEVICES,"MYDEVICE");
		
		if(retval)
		{
			#ifndef DEBUG
			printk(KERN_ALERT"registration unsuccessful\n");
			#endif
			goto OUT;
		}	
		
		else
		{
			major=MAJOR(dev);
			minor=MINOR(dev);
			#ifndef DEBUG
			printk(KERN_ALERT"registred the device with Major=%d Minor=%d\n",major,minor);
			#endif
		}
	}

	//allocation memory to sculldev & initializing this memory with NULL, as in kernel space kmalloc is used instead of malloc

	sculldev=kmalloc(NO_OF_DEVICES*sizeof(struct ScullDev),GFP_KERNEL);
	memset(sculldev,0,NO_OF_DEVICES*sizeof(struct ScullDev));
	
	if(sculldev)
	{	
		#ifndef DEBUG
		printk(KERN_ALERT"initialize sculldev memory\n");
		#endif
	}
	else
	{
		#ifndef DEBUG
		printk(KERN_ALERT"Failed to intialize sculldev memory\n");
		#endif
	}
	//callind cdev setup function to initialize & add the device, loops are supported on kernel level too.


	for(i=0;i<NO_OF_DEVICES;i++)
	{
		retval=scull_setup_cdev(sculldev,i);

		if(retval)
		{
			#ifndef DEBUG
			printk(KERN_ALERT"INITIALIZTION FAILED\n");
			#endif
			goto OUT;
		}	
		
		else
		{
			#ifndef DEBUG
			printk(KERN_ALERT"DEVICE INITIALIZED with \n");
			#endif
		
		}
	}
	
	
	#ifndef DEBUG
	printk(KERN_INFO"End:%s\n",__func__);
	#endif

	return 0;
OUT: return -1;
}
static void hello_exit(void)
{
	#ifndef DEBUG
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif

	cdev_del(&sculldev->cdev);
	kfree(sculldev);
	unregister_chrdev_region(dev,NO_OF_DEVICES);

	#ifndef DEBUG
	printk(KERN_INFO"End:%s\n",__func__);
	#endif
}
module_init(hello_init);
module_exit(hello_exit);

