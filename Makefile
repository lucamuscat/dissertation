ifdef SILENT
DEBUG_FLAGS = -g
else
DEBUG_FLAGS = -DDEBUG -g
endif
ERROR_FLAGS = -Wall -Wextra

FILTER_LOCK_FILES = ./locks/tests.c ./locks/util.c ./locks/test_locks.c ./locks/filter_lock.c
FILTER_LOCK_OUTPUT_NAME = filter_lock.o


filter_lock:
	gcc $(FILTER_LOCK_FILES) -lpthread -o locks/$(FILTER_LOCK_OUTPUT_NAME) $(DEBUG_FLAGS) $(ERROR_FLAGS)

run_filter_lock:
	./locks/filter_lock.o 10

debug_filter_lock:
	gdb --args ./locks/filter_lock.o 10