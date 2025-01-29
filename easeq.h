#if !defined(EASEQ_H)

#include <string.h>
#include <stdio.h>
#include <immintrin.h>
#include <windows.h>
#include <stdint.h>
#include <ctype.h>

#define NumberOfCPUCores 8
#define MAX_FILES 256 // Maximum number of files
#define MAX_PATH_LENGTH 512 // Maximum path length
#define NUM_CORES 8 // Number of cores to use
HANDLE global_handle_Mutex;

char global_SequencedFiles[MAX_FILES][MAX_PATH_LENGTH]; // Array of file paths
int global_FileCount = 0; // Number of files in the list
int global_FileAssigned[MAX_FILES] = { 0 }; // Flags to indicate whether a file is assigned

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef float f32;
typedef double f64;

typedef struct 
{
    char key;
    char value[2];
} KeyValuePair;

typedef struct 
{
    KeyValuePair entries[4];
} HashMap;

typedef struct {
    alignas(32) u64 data[32];
} AlignedArray;

#define EASEQ_H
#endif