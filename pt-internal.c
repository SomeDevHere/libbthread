#include <pthread.h>
#include <stdlib.h>
#include "pt-internal.h"

static pthread_key_t thread_key;
static pthread_once_t thread_key_once = PTHREAD_ONCE_INIT;

static struct pthread_internal_t* threads = NULL;
static struct pthread_internal_t* last_thread = NULL;

static pthread_mutex_t pthread_table_lock = PTHREAD_MUTEX_INITIALIZER;

struct pthread_internal_t* __pthread_lookup(pthread_t id) {
	struct pthread_internal_t* cur = threads;
	while (cur) {
		if(pthread_equal(cur->id, id)) {
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

// does not check if thread already exists
struct pthread_internal_t* __pthread_create(pthread_t id) {
	pthread_mutex_lock(&pthread_table_lock);
	struct pthread_internal_t* newthread = malloc(sizeof(struct pthread_internal_t));
	newthread->next = NULL;
	newthread->attr_flags = 0;
	newthread->id = id;
	pthread_mutex_init(&(newthread->cancel_lock), NULL);
	if (last_thread) {
		last_thread->next = newthread;
	} else {
		threads = newthread;
	}
	last_thread = newthread;
	pthread_mutex_unlock(&pthread_table_lock);
	return newthread;
}

void __pthread_remove(pthread_t id) {
	pthread_mutex_lock(&pthread_table_lock);
	struct pthread_internal_t* cur = threads;
	struct pthread_internal_t* prev = NULL;
	while (cur) {
		if(pthread_equal(cur->id, id)) {
			if (prev) {
				prev->next = cur->next;
			} else {
				threads = cur->next;
			}
			if (cur == last_thread) {
				last_thread = prev;
			}
			pthread_mutex_destroy(&(cur->cancel_lock));
			free(cur);
			pthread_mutex_unlock(&pthread_table_lock);
			return;
		}
		prev = cur;
		cur = cur->next;
	}
	pthread_mutex_unlock(&pthread_table_lock);
}

void __pthread_cleanup(void* key) {
	struct pthread_internal_t* self = (struct pthread_internal_t*) key;
	__pthread_remove(self->id);
}

static void pthread_create_key() {
	pthread_key_create(&thread_key, __pthread_cleanup);
}

struct pthread_internal_t* __pthread_internal_init() {
	pthread_once(&thread_key_once, pthread_create_key);
	struct pthread_internal_t* self = pthread_getspecific(thread_key);
	if (!self) {
		self = __pthread_create(pthread_self());
		pthread_setspecific(thread_key, self);
	}
	return self;
}

struct pthread_internal_t* __pthread_internal_self() {
	struct pthread_internal_t* self = pthread_getspecific(thread_key);
	return self;
}
