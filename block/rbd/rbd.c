/*
 * rbd - having fun with block device driver
 * 
 * (part of code learned from /drivers/block/brd.c)
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

#include "rbd.h"

#define DRIVER_DESC   "A block device driver for having fun"

#define SECTOR_SHIFT 9 

static struct rbd_device *rbd_dev;
static int max_part = 16;
module_param(max_part, int, 0);
static int nsectors = 10240;	/* 10240 sectors (20 MB) */
module_param(nsectors, int, 0);

static DEFINE_SPINLOCK(rbd_lock);

/*
 * handle an I/O request
 */
static int rbd_do_bvec(struct rbd_device *dev, 
						struct page *page,
						unsigned int len,
						unsigned int off,
						int rw,
						sector_t sector)
{
	if (rw == READ) 
		printk(KERN_INFO "[rbd]: Read request\n"); 
	else
		printk(KERN_INFO "[rbd]: WRITE request\n");

	return 0;
}

/*
 * request handler
 */ 
static int rbd_make_request(struct request_queue *q, struct bio *bio)
{
	struct block_device *bdev = bio->bi_bdev;
	struct rbd_device *rbd = bdev->bd_disk->private_data;
	int rw;
	struct bio_vec *bvec;
	sector_t sector;
	int i;
	int err = -EIO;

	sector = bio->bi_sector;
	if (sector + (bio->bi_size >> SECTOR_SHIFT) > 
						get_capacity(bdev->bd_disk))
		goto out;

	rw = bio_rw(bio);
	if (rw == READA)
		rw = READ;

	bio_for_each_segment(bvec, bio, i) {
		unsigned int len = bvec->bv_len;
		err = rbd_do_bvec(rbd, bvec->bv_page, len, 
					bvec->bv_offset, rw, sector);

		if (err)
			break;
		sector += len >> SECTOR_SHIFT;
	}
	
out:
	bio_endio(bio, err);

	return 0;
}

static int rbd_ioctl(struct rbd_device *bdev, fmode_t mode,
			unsigned int cmd, unsigned long arg)
{
	printk(KERN_INFO "blockdrive: receive command %u\n", cmd);
	return 0;
}

static const struct block_device_operations rbd_fops = 
{
	.owner = THIS_MODULE,
	.ioctl = rbd_ioctl,
};

static int __init rbd_init(void)
{
	int err = -ENOMEM;
	unsigned int major = UNNAMED_MAJOR;
	
	if( (major = register_blkdev(0, "blockdrive")) < 0){
		return -EIO;
	}
	printk(KERN_INFO "blockdrive: registered device at major %u\n", major);
	
	rbd_dev = kcalloc(1, sizeof(*rbd_dev), GFP_KERNEL);
	if(!rbd_dev){
		return -ENOMEM;
	}

	rbd_dev->blksize = 512;
    rbd_dev->nblocks = nsectors;
	rbd_dev->data = vmalloc(rbd_dev->blksize * rbd_dev->nblocks);
	if (rbd_dev->data == NULL) {
		kfree(rbd_dev);
		return -ENOMEM;
	}

	struct gendisk *disk = alloc_disk(max_part);
	if(!disk){
		goto out;
	}
	
	disk->major = major;
	disk->first_minor = 0;
	disk->fops = &rbd_fops;
	sprintf(disk->disk_name, "ramdisk%d", 0);
	disk->private_data = rbd_dev;
	disk->queue = blk_alloc_queue(GFP_KERNEL);
	if(!disk->queue){
		put_disk(disk);
		goto out;
	}
	blk_queue_make_request(disk->queue, rbd_make_request);

	add_disk(disk);
	rbd_dev->disk = disk;
	
  	return 0;
out:
	put_disk(disk);
	kfree(rbd_dev);
	return err;
}

static void __exit rbd_cleanup(void)
{
	struct gendisk *disk = rbd_dev->disk;
	unsigned major = disk->major;
	if(disk){
		del_gendisk(disk);
		blk_cleanup_queue(disk->queue);
		put_disk(disk);
	}
	unregister_blkdev(major, "blockdrive");
	vfree(rbd_dev->data);
	kfree(rbd_dev);
  	printk(KERN_INFO "blockdrive: unregistered device at major %u\n", major);
	
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);

module_init(rbd_init);
module_exit(rbd_cleanup);
