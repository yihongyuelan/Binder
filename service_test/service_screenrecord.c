/* Copyright 2008 The Android Open Source Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <private/android_filesystem_config.h>

#include "binder.h"

#if 0
#define ALOGI(x...) fprintf(stderr, "svcmgr: " x)
#define ALOGE(x...) fprintf(stderr, "svcmgr: " x)
#else
#define LOG_TAG "#h#j#"
#include <cutils/log.h>
#endif

void *svcmgr_handle;

const char *str8(uint16_t *x)
{
    static char buf[128];
    unsigned max = 127;
    char *p = buf;

    if (x) {
        while (*x && max--) {
            *p++ = *x++;
        }
    }
    *p++ = 0;
    return buf;
}

int str16eq(uint16_t *a, const char *b)
{
    while (*a && *b)
        if (*a++ != *b++) return 0;
    if (*a || *b)
        return 0;
    return 1;
}


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

    binder_done(bs, &msg, &reply);

    return status;
}
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

static pid_t pidt;
int svcmgr_handler(struct binder_state *bs,
                   struct binder_txn *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
    uint16_t *s;
    unsigned len,status=0;
    void *ptr;
	int fd=0;
    ALOGE("target=%p code=%d pid=%d uid=%d\n",
         txn->target, txn->code, txn->sender_pid, txn->sender_euid);

    switch(txn->code) {
		case 1:
			s = bio_get_string16(msg, &len);
			fd=open(str8(s),O_CREAT|O_WRONLY,666);
			if(fd > 0 && pidt == 0){
				close(fd);
				pidt = vfork();
				if(0==pidt){
					ALOGE("enter child process");
					status = execl("/system/bin/screenrecord","screenrecord",str8(s),(char*)0);
					ALOGE("exit status=%d",status);
					exit(0);
				}else if(pidt < 0){
					ALOGE("fork process fail");
				}else {
					ALOGE("cmd 1 start at s:%s  path:%s",s,str8(s));
				}
			}else{
				ALOGE("service have been started");
				status = -1;
				if(fd < 0) status = -2;
			}
			break;
		case 2:
			ALOGE("cmd 2 stop");
			if(pidt != 0){
				status = kill(pidt,2);
				wait(0);
				pidt = 0;
			}else{
				ALOGE("service have been killed");
			}	
			break;
		case 3:
			ALOGE("cmd 3 check sr state");
		    bio_put_uint32(reply, pidt != 0);//返回状态码
			break;
		default:
		    ALOGE("unknown code %d\n", txn->code);
			status=-1;
		    return -1;
    }

    bio_put_uint32(reply, status);//返回状态码
    return 0;
}

static unsigned binder_handle;//BINDER_TYPE_BINDER

int main(int argc, char **argv)
{
    struct binder_state *bs;

    bs = binder_open(128*1024);

	svcmgr_publish(bs,BINDER_SERVICE_MANAGER,"screenrecord",&binder_handle);//

    binder_loop(bs, svcmgr_handler);
    return 0;
}
