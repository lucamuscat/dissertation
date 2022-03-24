SHELL=bash

ifdef DEBUG
DEBUG_FLAGS = -DDEBUG -g
else
DEBUG_FLAGS = -O3 -g
endif
ERROR_FLAGS = -Wall -Wextra

ASM_FLAGS = -masm=intel -S -fno-exceptions -fno-dwarf2-cfi-asm -fno-asynchronous-unwind-tables -fno-exceptions
LIBRARIES = -lm -lpthread
# Libraries needed for the nonblocking builds
# mcx16 is needed to emit the cmpxchg16b instruction
NONBLOCKING_LIBRARIES = -lm -lpthread -latomic -march=native
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

# We are using clang as gcc does not emit cmpxchg16b during a double-width CAS
# (in my case anyways)
CXX = clang

all: sequential_latency_tests lock_contention_tests enqueue_dequeue_blocking_tests

sequential_latency_tests: sequential_latency_spin_lock_test sequential_latency_ttas_lock_test sequential_latency_pthread_lock_test

increment_counter_tests: increment_pthread_lock_test increment_spin_lock_test increment_ttas_lock_test

lock_contention_tests: lock_contention_pthread_lock_test lock_contention_spin_lock_test lock_contention_ttas_lock_test

init_build_folder:
	mkdir -p $(OUTPUT_DIR)

increment_%_test: init_build_folder
	$(CXX) $(LOCKS_DIR)/$*.c $(INCREMENT_TEST_FILES) $(LIBRARIES) $(PAPI_LIB) -o $(OUTPUT_DIR)/$*_inc.o $(DEBUG_FLAGS) $(ERROR_FLAGS)

SEQUENTIAL_LATENCY_COMMON = $(CXX) $(SEQUENTIAL_LATENCY_TEST_FILES) $(LOCKS_DIR)/$*.c $(LIBRARIES)

lock_contention_%_test: init_build_folder
	$(CXX) $(LOCKS_DIR)/$*.c $(LOCKS_DIR)/tests/contention.c $(TEST_UTILS) $(LIBRARIES) $(PAPI_LIB) $(DEBUG_FLAGS) $(ERROR_FLAGS) -o $(OUTPUT_DIR)/$*_contention

sequential_latency_%_test: init_build_folder
	$(SEQUENTIAL_LATENCY_COMMON) $(PAPI_LIB) -o $(OUTPUT_DIR)/$*_seq_lat.o $(DEBUG_FLAGS) $(ERROR_FLAGS)

QUEUES_DIR = $(SRC_DIR)/queues
BLOCKING_DIR = $(QUEUES_DIR)/blocking
NONBLOCKING_DIR = $(QUEUES_DIR)/non-blocking
ENQUEUE_DEQUEUE_TEST_FILES = $(QUEUES_DIR)/tests/enqueue_dequeue.c $(TEST_UTILS)
P_ENQUEUE_DEQUEUE_TEST_FILES = $(QUEUES_DIR)/tests/p_enqueue_dequeue.c $(TEST_UTILS)

enqueue_dequeue_blocking_tests: enqueue_dequeue_blocking_linked_queue_test

enqueue_dequeue_non_blocking_tests: enqueue_dequeue_nonblocking_ms_queue_test
p_enqueue_dequeue_non_blocking_tests: p_enqueue_dequeue_nonblocking_ms_queue_test


enqueue_dequeue_blocking_%_test: init_build_folder
	for i in $(LOCK_FILES); \
	do \
		$(CXX) $(BLOCKING_DIR)/$*.c $(ENQUEUE_DEQUEUE_TEST_FILES) $$i $(PAPI_LIB) $(LIBRARIES) -o $(OUTPUT_DIR)/blocking_$*_`basename $$i .c` $(DEBUG_FLAGS) $(ERROR_FLAGS); \
	done

enqueue_dequeue_nonblocking_%_test: init_build_folder
	$(CXX) $(NONBLOCKING_DIR)/$*.c $(ENQUEUE_DEQUEUE_TEST_FILES) $(PAPI_LIB) $(NONBLOCKING_LIBRARIES) -o $(OUTPUT_DIR)/nonblocking_$* $(DEBUG_FLAGS) $(ERROR_FLAGS)

p_enqueue_dequeue_blocking_tests: p_enqueue_dequeue_blocking_linked_queue_test

p_enqueue_dequeue_blocking_%_test: init_build_folder
	for i in $(LOCK_FILES); \
	do \
		$(CXX) $(BLOCKING_DIR)/$*.c $(P_ENQUEUE_DEQUEUE_TEST_FILES) $$i $(PAPI_LIB) $(LIBRARIES) -o $(OUTPUT_DIR)/p_blocking_$*_`basename $$i .c` $(DEBUG_FLAGS) $(ERROR_FLAGS); \
	done

COMMON_DELAY = $(CXX) $(QUEUES_DIR)/tests/delay_test.c $(TEST_UTILS) -lm  $(DEBUG_FLAGS) 
delay_test: init_build_folder
	$(COMMON_DELAY) $(PAPI_LIB) -o $(OUTPUT_DIR)/delay_test $(ERROR_FLAGS)
	$(COMMON_DELAY) $(ASM_FLAGS)

clean: 
	rm -rf ./build
	rm -f *.s *.o