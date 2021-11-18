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

INCREMENT_TEST_FILES = $(TESTS_DIR)/increment_counter/*.c
FILTER_LOCK_INCREMENT_OUTPUT_NAME = filter_lock_inc.o

all: increment_filter_lock_test increment_kernel_lock_test increment_peterson_lock_test

init_build_folder:
	mkdir -p $(OUTPUT_DIR)

increment_filter_lock_test: init_build_folder
	$(INIT_BUILD_FOLDER)
	mkdir -p ./build/asm/filter/
	gcc $(FILTER_LOCK_DEPENDENCIES) $(INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/$(FILTER_LOCK_INCREMENT_OUTPUT_NAME) $(DEBUG_FLAGS) $(ERROR_FLAGS)
	gcc $(FILTER_LOCK_DEPENDENCIES) $(INCREMENT_TEST_FILES) $(LIBRARIES) -masm=intel -S -fverbose-asm -fno-asynchronous-unwind-tables -fno-exceptions
	mv *.s build/asm/filter


increment_kernel_lock_test: init_build_folder
	gcc ./locks/kernel_lock.c $(INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/kernel_lock_inc.o $(DEBUG_FLAGS) $(ERROR_FLAGS)

increment_peterson_lock_test: init_build_folder
	mkdir -p ./build/asm/peterson/
	gcc ./locks/peterson_lock.c $(INCREMENT_TEST_FILES) $(LIBRARIES) -o $(OUTPUT_DIR)/peterson_lock_inc.o $(DEBUG_FLAGS) $(ERROR_FLAGS)
	gcc ./locks/peterson_lock.c $(INCREMENT_TEST_FILES) $(LIBRARIES) -masm=intel -S -fverbose-asm -fno-asynchronous-unwind-tables -fno-exceptions
	mv *.s build/asm/peterson/