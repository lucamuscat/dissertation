ifdef SILENT
DEBUG_FLAGS = -g
else
DEBUG_FLAGS = -DDEBUG -g
endif
ERROR_FLAGS = -Wall -Wextra

ASM_FLAGS = -masm=intel -S -fverbose-asm -fno-asynchronous-unwind-tables -fno-exceptions
LIBRARIES = -lpthread -fopenmp
TESTS_DIR = ./locks/tests
OUTPUT_DIR = ./build

FILTER_LOCK_DEPENDENCIES = ./locks/filter_lock.c

INCREMENT_TEST_FILES = $(TESTS_DIR)/increment_counter/*.c $(TESTS_DIR)/test_utils.c
SEQUENTIAL_LATENCY_TEST_FILES = $(TESTS_DIR)/sequential_latency.c .$(TESTS_DIR)/test_utils.c

SEQUENTIAL_LATENCY_TEST_FILES = $(TESTS_DIR)/sequential_latency.c $(TESTS_DIR)/test_utils.c
FILTER_LOCK_INCREMENT_OUTPUT_NAME = filter_lock_inc.o

CXX = gcc

all: increment_counter_tests sequential_latency_tests

sequential_latency_tests: sequential_latency_filter_lock_test sequential_latency_peterson_lock_test sequential_latency_spin_lock_test sequential_latency_kernel_lock_test

increment_counter_tests: increment_filter_lock_test increment_kernel_lock_test increment_peterson_lock_test increment_spin_lock_test

init_build_folder:
	mkdir -p $(OUTPUT_DIR)

increment_kernel_lock_test: init_build_folder
	$(CXX) ./locks/kernel_lock.c $(INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/kernel_lock_inc.o $(DEBUG_FLAGS) $(ERROR_FLAGS)

increment_%_test: init_build_folder
	mkdir -p ./build/asm/increment/$*/
	$(CXX) ./locks/$*.c $(INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/$*_inc.o $(DEBUG_FLAGS) $(ERROR_FLAGS)
	$(CXX) ./locks/$*.c $(INCREMENT_TEST_FILES) $(LIBRARIES) $(ASM_FLAGS)
	mv *.s build/asm/increment/$*

sequential_latency_%_test: init_build_folder
	mkdir -p ./build/asm/sequential_latency/$*/
	$(CXX) $(SEQUENTIAL_LATENCY_TEST_FILES) ./locks/$*.c  -lrt $(LIBRARIES)  -o $(OUTPUT_DIR)/$*_seq_lat.o $(DEBUG_FLAGS) $(ERROR_FLAGS)
	$(CXX) $(SEQUENTIAL_LATENCY_TEST_FILES) ./locks/$*.c  -lrt $(LIBRARIES) $(ASM_FLAGS)
	mv *.s build/asm/sequential_latency/$*

clean: 
	rm -rf ./build
	rm *.s *.o