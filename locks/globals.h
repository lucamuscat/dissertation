#ifndef GLOBALS_H
#define GLOBALS_H

// https://stackoverflow.com/a/7246573/7788102
#ifdef DEBUG 
#define DEBUG_LOG(message) printf(message)
#define DEBUG_LOG_F(message, format) printf(message, format)
#else
#define DEBUG_LOG(message) do {} while(0)
#define DEBUG_LOG_F(message, format) do {} while(0)
#endif

#define FULL_BARRIER asm("mfence")
// Untested
#define WRITE_BARRIER asm("sfence")
#define LOAD_BARRIER asm("lfence")

#endif