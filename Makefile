filter_lock:
	gcc ./locks/util.c ./locks/test_locks.c ./locks/filter_lock.c -lpthread -o locks/filter_lock.o -g -Wall -Wextra