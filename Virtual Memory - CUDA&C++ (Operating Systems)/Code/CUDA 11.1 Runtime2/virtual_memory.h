#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include <cuda.h>
#include <cuda_runtime.h>
#include <inttypes.h>

typedef unsigned char uchar;
typedef uint32_t u32;

struct node {
    int physical_address = 0;   // position in nodes
    int counter = INT_MAX;      // sort according to counter
    int position = 0;           // position in LRU
};

struct VirtualMemory {
    uchar *buffer;
    uchar *storage;
    u32 *invert_page_table;
    int *pagefault_num_ptr;

    int PAGESIZE;
    int INVERT_PAGE_TABLE_SIZE;
    int PHYSICAL_MEM_SIZE;
    int STORAGE_SIZE;
    int PAGE_ENTRIES;

    node* nodes[1024];  // store all the nodes in LRU, help to modify counters
    node* LRU[1025];     // using min heap to extact the node with the min count, start store from 1
    int count = 0;       // time count
public:
    int pointer = 0;     // pointer from 0 to 1024
    int pid = 0;
};

// TODO
__device__ void perlocateUp(node* array[], int hole);
__device__ void perlocateDown(node* array[], int hole);
__device__ void vm_init(VirtualMemory *vm, uchar *buffer, uchar *storage,
                        u32 *invert_page_table, int *pagefault_num_ptr,
                        int PAGESIZE, int INVERT_PAGE_TABLE_SIZE,
                        int PHYSICAL_MEM_SIZE, int STORAGE_SIZE,
                        int PAGE_ENTRIES);
__device__ uchar vm_read(VirtualMemory *vm, u32 addr);
__device__ void vm_write(VirtualMemory *vm, u32 addr, uchar value);
__device__ void vm_snapshot(VirtualMemory *vm, uchar *results, int offset,
                            int input_size);

#endif
