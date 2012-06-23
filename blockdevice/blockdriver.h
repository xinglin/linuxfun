/*
 * 2012 Copyright (C) note.
 * Data structure is learned from /include/linux/nbd.h
 */
#ifndef LINUX_BLOCKDRIVER_H
#define LINUX_BLOCKDRIVER_H

#include <linux/types.h>

enum {
	CD_CMD_READ = 0,
	CD_CMD_WRITE = 1,
	CD_CMD_DISC = 2
};

struct expr_device {
	int flags;
	int magic;
	
	spinlock_t queue_lock;
	struct list_head queue_head;	/* Requests waiting result */
	struct list_head waiting_queue;	/* Requests to be sent */
	
	struct gendisk *disk;
	int blksize;
	u64 bytesize;
	pid_t pid;
	int xmit_timeout;
};

#endif
