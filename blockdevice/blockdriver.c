/*
 * blockDriver - having fun with block device driver
 * 
 * (part of code learned from /drivers/block/nbd.c)
 */
 
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/bio.h>
#include <linux/ioctl.h>

#include "blockdriver.h"

#define DRIVER_DESC   "A block device driver for having fun"

static struct expr_device *block_dev;
static int max_part = 16;
static DEFINE_SPINLOCK(blockdrive_lock);

/*
 * request handler
 */ 
static void do_blockdrive_request(struct request_queue *q)
{
	
}

static int blockdrive_ioctl(struct block_device *bdev, fmode_t mode,
			unsigned int cmd, unsigned long arg)
{
	return 0;
}
static const struct block_device_operations blockdrive_fops = 
{
	.owner = THIS_MODULE,
	.ioctl = blockdrive_ioctl,
};

static int __init block_init(void)
{
	int err = -ENOMEM;
	unsigned int major = UNNAMED_MAJOR;
	
	if( (major = register_blkdev(0, "blockdrive")) < 0){
		err = -EIO;
		return err;
	}
	printk(KERN_INFO "blockdrive: registered device at major %u\n", major);
	
	block_dev = kcalloc(1, sizeof(*block_dev), GFP_KERNEL);
	if(!block_dev){
		return -ENOMEM;
	}
	
	struct gendisk *disk = alloc_disk(max_part);
	if(!disk){
		goto out;
	}
	
	disk->major = major;
	disk->first_minor = 0;
	disk->fops = &blockdrive_fops;
	sprintf(disk->disk_name, "blockdrive%d", 0);
	disk->queue = blk_init_queue(do_blockdrive_request, 
				&blockdrive_lock);
	if(!disk->queue){
		put_disk(disk);
		goto out;
	}
	add_disk(disk);
	block_dev->disk = disk;
	
  	return 0;
out:
	put_disk(disk);
	kfree(block_dev);
	return err;
}

static void __exit block_cleanup(void)
{
	struct gendisk *disk = block_dev->disk;
	unsigned major = disk->major;
	if(disk){
		del_gendisk(disk);
		blk_cleanup_queue(disk->queue);
		put_disk(disk);
	}
	unregister_blkdev(major, "blockdrive");
	kfree(block_dev);
  	printk(KERN_INFO "blockdrive: unregistered device at major %u\n", major);
	
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);

module_init(block_init);
module_exit(block_cleanup);
