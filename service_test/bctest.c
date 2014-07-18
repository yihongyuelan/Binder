/* Copyright 2008 The Android Open Source Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "binder.h"
#define LOG_TAG "#h#j#_bctest"
#include <cutils/log.h>
void *svcmgr_lookup(struct binder_state *bs, void *target, const char *name)
{
    void *ptr;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_CHECK_SERVICE))
        return 0;

    ptr = bio_get_ref(&reply);

    if (ptr)
        binder_acquire(bs, ptr);

    binder_done(bs, &msg, &reply);

    return ptr;
}

int svcmgr_publish(struct binder_state *bs, void *target, const char *name, void *ptr)
{
    unsigned status;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);
    bio_put_obj(&msg, ptr);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_ADD_SERVICE))
        return -1;

    status = bio_get_uint32(&reply);
	ALOGE("return %d\n",status);
    binder_done(bs, &msg, &reply);

    return status;
}

int sr_cmd(struct binder_state *bs, void *target, unsigned cmd,void*path)
{
    void *ptr;
    unsigned iodata[512/4];
	int status;
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);

	if(path)
		bio_put_string16_x(&msg,path);
	

    if (binder_call(bs, &msg, &reply, target,cmd))
        return 0;

    status = bio_get_uint32(&reply);
	ALOGE("sr_cmd return %d\n",status);
    binder_done(bs, &msg, &reply);//释放内核reply来的buffer

    return status;
}

int main(int argc, char **argv)
{
    int fd;
    struct binder_state *bs;
    void *svcmgr = BINDER_SERVICE_MANAGER;

    bs = binder_open(128*1024);

	void *ptr = svcmgr_lookup(bs, svcmgr, "screenrecord");
	sr_cmd(bs,ptr,argv[1][0]-'0',argv[2]);
    return 0;
}
