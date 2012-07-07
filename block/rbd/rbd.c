/*
 * rbd - simple ram based block device
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
#include <linux/hdreg.h>

#include "rbd.h"

#define DRIVER_DESC   "simple ram based block device"

#define SECTOR_SHIFT 9 
#define KERNEL_SECTOR_SIZE 512

static struct rbd_device dev;
static int max_part = 16;
module_param(max_part, int, 0);
static int nsectors = 10240;	/* 10240 sectors (20 MB) */
module_param(nsectors, int, 0);

static struct request_queue *Queue;
/*
 * handle an I/O request
 */
static void sbd_do_transfer(struct rbd_device *pdev, 
						sector_t sector,
						unsigned int nsect,
						char *buffer,
						int write)
{
	unsigned long offset = sector * pdev->blksize;
	unsigned long nbytes = nsect * pdev->blksize;

	if ((offset + nbytes) > pdev->blksize * pdev->nblocks) {
		printk(KERN_NOTICE "rbd: Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}

	if (write) {
		//printk(KERN_INFO "rbd: READ (%ld %ld)\n", offset, nbytes);
		memcpy(pdev->data + offset, buffer, nbytes);
	} else {
		//printk(KERN_INFO "rbd: WRITE (%ld %ld)\n", offset, nbytes);
		memcpy(buffer, pdev->data + offset, nbytes);
	}
}

/*
 * request handler
 */ 
static void sbd_make_request(struct request_queue *q)
{
	struct request *req;

	req = blk_fetch_request(q);
	while (req != NULL) {
		if (req->cmd_type != REQ_TYPE_FS) {
			printk (KERN_NOTICE "rbd: Skip non-CMD request\n");
			__blk_end_request_all(req, -EIO);
			continue;
		}

		sbd_do_transfer(&dev, blk_rq_pos(req), blk_rq_cur_sectors(req), 
						req->buffer, rq_data_dir(req));
		if (!__blk_end_request_cur(req, 0)) {
			req = blk_fetch_request(q);
		}
	}
}

int sbd_getgeo(struct block_device *bdev, struct hd_geometry * geo)
{
	long size;

	/* We have no real geometry, of course, so make something up. */
	size = dev.blksize * dev.nblocks * (dev.blksize / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & 0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	return 0;
}

static const struct block_device_operations rbd_ops = 
{
	.owner = THIS_MODULE,
	.getgeo = sbd_getgeo,
};

static int __init rbd_init(void)
{
	unsigned int major = UNNAMED_MAJOR;
	
	dev.blksize = 512;
	dev.nblocks = nsectors;
	spin_lock_init(&dev.queue_lock);
	dev.data = vmalloc(dev.blksize * dev.nblocks);
	if (dev.data == NULL)
		return -ENOMEM;

	Queue = blk_init_queue(sbd_make_request, &dev.queue_lock);
	if (Queue == NULL)
		goto out;
	blk_queue_logical_block_size(Queue, dev.blksize);

	if ((major = register_blkdev(0, "rbd")) < 0){
		printk(KERN_WARNING "rbd: Unable to get major number\n");
		goto out;
	}
	printk(KERN_INFO "rbd: registered device at major %u\n", major);
	dev.disk = alloc_disk(max_part);
	if (dev.disk == NULL)
		goto out_unregister;
	
	dev.disk->major = major;
	dev.disk->first_minor = 0;
	dev.disk->fops = &rbd_ops;
	dev.disk->private_data = &dev;
	strcpy(dev.disk->disk_name, "rbd0");
	set_capacity(dev.disk, nsectors);
	dev.disk->queue = Queue;
	add_disk(dev.disk);
	
  	return 0;

out_unregister:
	unregister_blkdev(major, "rbd");
out:
	vfree(dev.data);
	return -ENOMEM;
}

static void __exit rbd_cleanup(void)
{
	struct gendisk *disk = dev.disk;
	unsigned major = disk->major;
	if(disk){
		del_gendisk(disk);
		blk_cleanup_queue(disk->queue);
		put_disk(disk);
	}
	unregister_blkdev(major, "rbd");
	vfree(dev.data);
  	printk(KERN_INFO "rbd: Unregistered device at major %u\n", major);
	
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);

module_init(rbd_init);
module_exit(rbd_cleanup);
