SHELL=bash

ifdef SILENT
DEBUG_FLAGS = -O3
else
DEBUG_FLAGS = -DDEBUG -g
endif
ERROR_FLAGS = -Wall -Wextra

ASM_FLAGS = -masm=intel -S -fverbose-asm -fno-asynchronous-unwind-tables -fno-exceptions
LIBRARIES = -lm -lpthread
PAPI_LIB = /usr/local/lib/libpapi.a 

SRC_DIR = ./src
LOCKS_DIR = $(SRC_DIR)/locks
LOCKS_TESTS_DIR = $(LOCKS_DIR)/tests
TEST_UTILS = $(SRC_DIR)/test_utils.c
OUTPUT_DIR = ./build

FILTER_LOCK_DEPENDENCIES = ./locks/filter_lock.c

INCREMENT_TEST_FILES = $(LOCKS_TESTS_DIR)/increment_counter/*.c $(TEST_UTILS)
SEQUENTIAL_LATENCY_TEST_FILES = $(LOCKS_TESTS_DIR)/sequential_latency.c $(TEST_UTILS)

SEQUENTIAL_LATENCY_TEST_FILES = $(LOCKS_TESTS_DIR)/sequential_latency.c $(TEST_UTILS)
FILTER_LOCK_INCREMENT_OUTPUT_NAME = filter_lock_inc.o

LOCK_FILES = $(LOCKS_DIR)/pthread_lock.c $(LOCKS_DIR)/spin_lock.c $(LOCKS_DIR)/ttas_lock.c

CXX = gcc

all: sequential_latency_tests

sequential_latency_tests: sequential_latency_spin_lock_test sequential_latency_kernel_lock_test sequential_latency_ttas_lock_test

increment_counter_tests: increment_filter_lock_test increment_kernel_lock_test increment_spin_lock_test increment_ttas_lock_test

init_build_folder:
	mkdir -p $(OUTPUT_DIR)

increment_kernel_lock_test: init_build_folder
	$(CXX) $(LOCKS_DIR)/kernel_lock.c $(INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/kernel_lock_inc.o $(DEBUG_FLAGS) $(ERROR_FLAGS)

increment_%_test: init_build_folder
	mkdir -p ./build/asm/increment/$*/
	$(CXX) $(LOCKS_DIR)/$*.c $(INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/$*_inc.o $(DEBUG_FLAGS) $(ERROR_FLAGS)
	$(CXX) $(LOCKS_DIR)/$*.c $(INCREMENT_TEST_FILES) $(LIBRARIES) $(ASM_FLAGS)
	mv *.s build/asm/increment/$*

SEQUENTIAL_LATENCY_COMMON = $(CXX) $(SEQUENTIAL_LATENCY_TEST_FILES) $(LOCKS_DIR)/$*.c $(LIBRARIES)

sequential_latency_%_test: init_build_folder
	mkdir -p ./build/asm/sequential_latency/$*/
	$(SEQUENTIAL_LATENCY_COMMON) $(PAPI_LIB) -o $(OUTPUT_DIR)/$*_seq_lat.o $(DEBUG_FLAGS) $(ERROR_FLAGS)
	$(SEQUENTIAL_LATENCY_COMMON) $(ASM_FLAGS)
	mv *.s build/asm/sequential_latency/$*

QUEUES_DIR = $(SRC_DIR)/queues
BLOCKING_DIR = $(QUEUES_DIR)/blocking
NONBLOCKING_DIR = $(QUEUES_DIR)/non-blocking
ENQUEUE_DEQUEUE_TEST_FILES = $(QUEUES_DIR)/tests/enqueue_dequeue.c $(TEST_UTILS)

enqueue_dequeue_blocking_tests: enqueue_dequeue_blocking_circular_buffer_test

enqueue_dequeue_blocking_%_test: init_build_folder
	for i in $(LOCK_FILES); \
	do \
		$(CXX) $(BLOCKING_DIR)/$*.c $(ENQUEUE_DEQUEUE_TEST_FILES) $$i $(PAPI_LIB) $(LIBRARIES) -o $(OUTPUT_DIR)/blocking_$*_`basename $$i .c` $(DEBUG_FLAGS) $(ERROR_FLAGS); \
	done

enqueue_dequeue_nonblocking_%_test: init_build_folder
	$(CXX) $(NONBLOCKING_DIR)/$*.c $(ENQUEUE_DEQUEUE_TEST_FILES) $(PAPI_LIB) $(LIBRARIES) -o $(OUTPUT_DIR)/nonblocking_$* $(DEBUG_FLAGS) $(ERROR_FLAGS)

clean: 
	rm -rf ./build
	rm -f *.s *.o