ifdef SILENT
DEBUG_FLAGS = -g
else
DEBUG_FLAGS = -DDEBUG -g
endif
ERROR_FLAGS = -Wall -Wextra

LIBRARIES = -lpthread -fopenmp
TESTS_DIR = ./locks/tests
OUTPUT_DIR = ./build

FILTER_LOCK_DEPENDENCIES = ./locks/filter_lock.c

FILTER_LOCK_INCREMENT_TEST_FILES = $(TESTS_DIR)/increment_counter/*.c
FILTER_LOCK_INCREMENT_OUTPUT_NAME = filter_lock_inc.o

all: increment_filter_lock_test

increment_filter_lock_test:
	gcc $(FILTER_LOCK_DEPENDENCIES) $(FILTER_LOCK_INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/$(FILTER_LOCK_INCREMENT_OUTPUT_NAME) $(DEBUG_FLAGS) $(ERROR_FLAGS)

increment_kernel_lock_test:
	gcc ./locks/kernel_lock.c $(FILTER_LOCK_INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/kernel_lock_inc.o $(DEBUG_FLAGS) $(ERROR_FLAGS)