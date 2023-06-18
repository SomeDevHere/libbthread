/* Tests
   Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <pt-internal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_request_lock = PTHREAD_MUTEX_INITIALIZER;
int print_request,want_exit,child_ready;

void print_pthread_internal(struct pthread_internal_t * p) {
	pthread_mutex_lock(&print_lock);
	
	printf("\n\n%s called by %p\n", __func__, (struct pthread_internal_t *)pthread_self());
	printf("p=%p\n",p);
	printf("p->next=%p\n", p->next);
	printf("p->attr.flags=%04x\n", p->attr_flags);
	
	pthread_mutex_unlock(&print_lock);
}


void *child(void *arg) {
	__pthread_init();
	child_ready = 1;

	pthread_t id = pthread_self();
	struct pthread_internal_t * p = __pthread_internal_self();
	int print,run;
	
	printf("child=%p\n", id);
	
	
	for(run=1;run;) {
		pthread_mutex_lock(&print_request_lock);
		if(print_request) {
			print_pthread_internal(p);
			print_request = 0;
		}
		run=!want_exit;
		pthread_mutex_unlock(&print_request_lock);
		
		usleep(100);
	}
}

void set_print(void) {
	pthread_mutex_lock(&print_request_lock);
	print_request = 1;
	pthread_mutex_unlock(&print_request_lock);
}

void wait_print(void) {
	int has_printed;
	do {
		usleep(100);
		pthread_mutex_lock(&print_request_lock);
		has_printed = (print_request==0);
		pthread_mutex_unlock(&print_request_lock);
	}while(!has_printed);
}

void wait_ready(void) {
	int is_ready;
	do {
		usleep(100);
		is_ready = (child_ready==1);
	}while(!is_ready);
}

int main(int argc, char **argv) {
	__pthread_init();
	
	pthread_t id,child_id;
	struct pthread_internal_t *p, *child_p;
	
	p = __pthread_internal_self();

	id=pthread_self();
	
	printf("main=%p\n", p);
	
	print_request = 1;
	want_exit = 0;
	child_ready = 0;
	
	pthread_create(&child_id, NULL, &child, NULL);
	
	wait_ready();

	child_p  = __pthread_lookup(child_id);
	
	wait_print();
	
	printf("setting cancel enable on %p\n", child_p);
	child_p->attr_flags |= PTHREAD_ATTR_FLAG_CANCEL_ENABLE;
	print_pthread_internal(child_p);
	
	set_print();
	wait_print();
	
	printf("setting cancel async on %p\n", child_p);
	child_p->attr_flags |= PTHREAD_ATTR_FLAG_CANCEL_ASYNCRONOUS;
	print_pthread_internal(child_p);
	
	set_print();
	wait_print();
	
	printf("setting cancel pending on %p\n", child_p);
	child_p->attr_flags |= PTHREAD_ATTR_FLAG_CANCEL_PENDING;
	print_pthread_internal(child_p);
	
	pthread_mutex_lock(&print_request_lock);
	print_request=want_exit=1;
	pthread_mutex_unlock(&print_request_lock);
	
	pthread_join(child_id, NULL); 
	
	return EXIT_SUCCESS;
}
