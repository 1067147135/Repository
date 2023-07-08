#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include "ioc_hw5.h"

MODULE_LICENSE("GPL");

#define PREFIX_TITLE "OS_AS5"

// DEVICE
static int dev_major;
static int dev_minor;
static struct cdev *dev_cdev;

// DMA
#define DMA_BUFSIZE 64
#define DMASTUIDADDR 0x0		 // Student ID
#define DMARWOKADDR 0x4			 // RW function complete
#define DMAIOCOKADDR 0x8		 // ioctl function complete
#define DMAIRQOKADDR 0xc		 // ISR function complete
#define DMACOUNTADDR 0x10		 // interrupt count function complete
#define DMAANSADDR 0x14			 // Computation answer
#define DMAREADABLEADDR 0x18 // READABLE variable for synchronize
#define DMABLOCKADDR 0x1c		 // Blocking or non-blocking IO
#define DMAOPCODEADDR 0x20	 // data.a opcode
#define DMAOPERANDBADDR 0x21 // data.b operand1
#define DMAOPERANDCADDR 0x25 // data.c operand2
void *dma_buf;

// Declaration for file operations
static ssize_t drv_read(struct file *filp, char __user *buffer, size_t, loff_t *);
static int drv_open(struct inode *, struct file *);
static ssize_t drv_write(struct file *filp, const char __user *buffer, size_t, loff_t *);
static int drv_release(struct inode *, struct file *);
static long drv_ioctl(struct file *, unsigned int, unsigned long);

// cdev file_operations
static struct file_operations fops = {
	owner : THIS_MODULE,
	read : drv_read,
	write : drv_write,
	unlocked_ioctl : drv_ioctl,
	open : drv_open,
	release : drv_release,
};

// in and out function
void myoutc(unsigned char data, unsigned short int port);	 // set char data
void myouts(unsigned short data, unsigned short int port); // set short data
void myouti(unsigned int data, unsigned short int port);	 // set int data
unsigned char myinc(unsigned short int port);							 // get char data
unsigned short myins(unsigned short int port);						 // get short data
unsigned int myini(unsigned short int port);							 // get int data

// Work routine
static struct work_struct *work_routine;

// For input data structure
struct DataIn
{
	char a;	 // operator: +, -, *, /, p(find prime number)
	int b;	 // operand 1
	short c; // operand 2
} * dataIn;

// IRQ
#define IRQ_NUM 1
static int counter = 0;

// Arithmetic funciton
static void drv_arithmetic_routine(struct work_struct *ws);

// Input and output data from/to DMA
void myoutc(unsigned char data, unsigned short int port)
{
	*(volatile unsigned char *)(dma_buf + port) = data;
}
void myouts(unsigned short data, unsigned short int port)
{
	*(volatile unsigned short *)(dma_buf + port) = data;
}
void myouti(unsigned int data, unsigned short int port)
{
	*(volatile unsigned int *)(dma_buf + port) = data;
}
unsigned char myinc(unsigned short int port)
{
	return *(volatile unsigned char *)(dma_buf + port);
}
unsigned short myins(unsigned short int port)
{
	return *(volatile unsigned short *)(dma_buf + port);
}
unsigned int myini(unsigned short int port)
{
	return *(volatile unsigned int *)(dma_buf + port);
}

static int drv_open(struct inode *ii, struct file *ff)
{
	try_module_get(THIS_MODULE);
	printk("%s:%s(): device open\n", PREFIX_TITLE, __func__);
	return 0;
}
static int drv_release(struct inode *ii, struct file *ff)
{
	module_put(THIS_MODULE);
	printk("%s:%s(): device close\n", PREFIX_TITLE, __func__);
	return 0;
}
static ssize_t drv_read(struct file *filp, char __user *buffer, size_t ss, loff_t *lo)
{
	/* Implement read operation for your device */
	int result = myini(DMAANSADDR);
	put_user(result, (int *)buffer); // Write a simple variable into user space.
	printk("%s:%s(): ans = %d\n", PREFIX_TITLE, __func__, result);
	return 0;
}
static ssize_t drv_write(struct file *filp, const char __user *buffer, size_t ss, loff_t *lo)
{ // buffer = data, ss = size of data
	/* Implement write operation for your device */
	
	dataIn = kmalloc(ss, GFP_KERNEL);
	raw_copy_from_user(dataIn, buffer, ss); // Copy a block of data from user space.
	myoutc(dataIn->a, DMAOPCODEADDR);				// Put opcode and operands into DMA buffer
	myouti(dataIn->b, DMAOPERANDBADDR);
	myouts(dataIn->c, DMAOPERANDCADDR);

	int IOMode;
	IOMode = myini(DMABLOCKADDR); // Blocking or non-blocking IO: 1->blocking, 0->non-blocking
	INIT_WORK(work_routine, drv_arithmetic_routine); // initialize work from an allocated buffer
	if (IOMode)
	{
		// Blocking IO
		printk("%s:%s(): queue work\n", PREFIX_TITLE, __func__);
		printk("%s:%s(): block\n", PREFIX_TITLE, __func__);
		schedule_work(work_routine); // put work task in global workqueue
		flush_scheduled_work();			 // flush work on work queue
	}
	else
	{
		// Non-locking IO
		printk("%s:%s(): queue work\n", PREFIX_TITLE, __func__);
		schedule_work(work_routine);
	}

	return 0;
}
static long drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	/* Implement ioctl setting for your device */
	int ret;
	get_user(ret, (int *)arg); // Get a simple variable from user space.

	if (cmd == HW5_IOCSETSTUID)		// 1
	{								// set student ID: printk you student ID
		myouti(ret, DMASTUIDADDR);	// Student ID
		printk("%s:%s(): My STUID is = %d\n", PREFIX_TITLE, __func__, ret); // ret = 119010265
	}
	else if (cmd == HW5_IOCSETRWOK)	// 2
	{								// set if RW OK: printk OK if you complete R/W function
		myouti(ret, DMARWOKADDR);	// RW function complete
		printk("%s:%s(): RW OK\n", PREFIX_TITLE, __func__);
	}
	else if (cmd == HW5_IOCSETIOCOK)// 3
	{								// set if ioctl OK: printk OK if you complete ioctl function
		myouti(ret, DMAIOCOKADDR);	// ioctl function complete
		printk("%s:%s(): IOC OK\n", PREFIX_TITLE, __func__);
	}
	else if (cmd == HW5_IOCSETIRQOK)// 4
	{								// set if IRQ OK: printk OK if you complete bonus
		myouti(ret, DMAIRQOKADDR); 	// ISR function complete
		printk("%s:%s(): IRQ OK\n", PREFIX_TITLE, __func__);
	}
	else if (cmd == HW5_IOCSETBLOCK)// 5
	{ 								// set blocking or non-blocking: set write function mode
		if (ret == 1)				// Blocking
		{
			myouti(1, DMABLOCKADDR); 
			printk("%s:%s(): Blocking IO\n", PREFIX_TITLE, __func__);
		}
		else if (ret == 0)			// Non-blocking IO
		{
			myouti(0, DMABLOCKADDR); 
			printk("%s:%s(): Non-Blocking IO\n", PREFIX_TITLE, __func__);
		}
	}
	else if (cmd == HW5_IOCWAITREADABLE)	// 6
	{								// Wait if readable now (synchronize function):
		printk("%s:%s(): wait readable 1\n", PREFIX_TITLE, __func__);
		int readable = 0; 			// used before read to confirm it can read answer noe when use non-blocking write mode
		while (!readable)
		{ 							// wait until the calculation result is readable
			msleep(200);
			readable = myini(DMAREADABLEADDR);
		}
		put_user(readable, (int *)arg);
		// printk("%s:%s(): wait readable %d\n", PREFIX_TITLE, __func__, readable);
	}

	return 0;
}

static void drv_arithmetic_routine(struct work_struct *ws)
{
	/* Implement arthemetic routine */
	char opcode = myinc(DMAOPCODEADDR);
	int operand1 = myini(DMAOPERANDBADDR);
	short operand2 = myins(DMAOPERANDCADDR);
	int result;

	myouti(0, DMAREADABLEADDR); // 0->unreadable, 1->readable

	if (opcode == '+')
	{
		result = operand1 + operand2;
	}
	else if (opcode == '-')
	{
		result = operand1 - operand2;
	}
	else if (opcode == '*')
	{
		result = operand1 * operand2;
	}
	else if (opcode == '/')
	{
		result = operand1 / operand2;
	}
	else if (opcode == 'p')
	{
		short count = 0;
		result = operand1;
		while (count < operand2)
		{
			bool flag = true; // true: is prime
			result++;
			int i = 2;
			for (; i < result / 2; i++)
			{
				if (result % i == 0)
				{
					flag = false;
					break;
				}
			}
			if (flag)
			{
				count++;
			}
		}
	}

	myouti(result, DMAANSADDR);
	myouti(1, DMAREADABLEADDR);
	printk("%s:%s(): %d %c %d = %d\n", PREFIX_TITLE, __func__, operand1, opcode, operand2, result);
}

irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{ // irq handler
	counter++;
	return (irq_handler_t)IRQ_HANDLED;
}

static int __init init_modules(void)
{
	dev_t dev;
	int ret_irq;
	int ret;
	printk("%s:%s():...............Start...............\n", PREFIX_TITLE, __func__);

	/* Register chrdev */
	ret = alloc_chrdev_region(&dev, 0, 1, "mydev");
	if (ret)
	{
		printk("Cannot alloc chrdev\n");
		return ret;
	}

	dev_major = MAJOR(dev);
	dev_minor = MINOR(dev);
	printk("%s:%s(): register chrdev(%d,%d)\n", PREFIX_TITLE, __func__, dev_major, dev_minor);

	ret_irq = request_irq(IRQ_NUM, (irq_handler_t)irq_handler, IRQF_SHARED, "keyboard_irq_state", (void *)(irq_handler));
	if (ret_irq)
	{
		printk("Request_irq failed.\n");
		return ret_irq;
	}
	printk("%s:%s(): request_irq %d return %d\n", PREFIX_TITLE, __FUNCTION__, IRQ_NUM, ret_irq);

	dev_cdev = cdev_alloc();

	/* Init cdev and make it alive */
	cdev_init(dev_cdev, &fops);
	dev_cdev->owner = THIS_MODULE;
	ret = cdev_add(dev_cdev, MKDEV(dev_major, dev_minor), 1);
	if (ret)
	{
		printk("Add cdev failed!\n");
		return ret;
	}
	/* Allocate DMA buffer */
	dma_buf = kzalloc(DMA_BUFSIZE, GFP_KERNEL); // kzalloc = kmalloc + memset
	printk("%s:%s(): allocate dma buffer\n", PREFIX_TITLE, __func__);

	/* Allocate work routine */
	work_routine = kmalloc(sizeof(typeof(*work_routine)), GFP_KERNEL);

	return 0;
}

static void __exit exit_modules(void)
{
	/* Print irq count and free irq */
	printk("%s:%s(): interrupt count = %d\n", PREFIX_TITLE, __FUNCTION__, counter);
	free_irq(IRQ_NUM, (void *)(irq_handler));

	/* Free DMA buffer when exit modules */
	kfree(dma_buf);
	printk("%s:%s(): free dma buffer\n", PREFIX_TITLE, __func__);

	/* Delete character device */
	unregister_chrdev_region(MKDEV(dev_major, dev_minor), 1);
	cdev_del(dev_cdev);
	printk("%s:%s(): unregister chrdev\n", PREFIX_TITLE, __func__);

	/* Free work routine */
	kfree(work_routine);

	printk("%s:%s():..............End..............\n", PREFIX_TITLE, __func__);
}

module_init(init_modules);
module_exit(exit_modules);


