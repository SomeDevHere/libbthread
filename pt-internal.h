#ifndef _PTHREAD_INTERNAL_H_
#define _PTHREAD_INTERNAL_H_
#include <pthread.h>
struct pthread_internal_t {
	pthread_t id;
	pthread_mutex_t cancel_lock;
	int attr_flags;
	struct pthread_internal_t* next;
};

struct pthread_internal_t* __pthread_internal_init();
struct pthread_internal_t* __pthread_internal_self();
struct pthread_internal_t* __pthread_lookup(pthread_t);

void __pthread_init();

int __pthread_do_cancel(struct pthread_internal_t*, pthread_t);


#define PTHREAD_ATTR_FLAG_CANCEL_HANDLER 0x1
#define PTHREAD_ATTR_FLAG_CANCEL_PENDING 0x2
#define PTHREAD_ATTR_FLAG_CANCEL_ENABLE 0x4
#define PTHREAD_ATTR_FLAG_CANCEL_ASYNCRONOUS 0x8

#endif /* _PTHREAD_INTERNAL_H_ */
