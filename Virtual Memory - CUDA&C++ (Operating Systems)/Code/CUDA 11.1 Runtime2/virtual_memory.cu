#include "virtual_memory.h"
#include <cuda.h>
#include <cuda_runtime.h>

__device__ void perlocateUp(node* array[], int hole) {
    int parent;
    node* tmp = array[hole];
    while (hole / 2 > 0) {
        parent = hole / 2;
        if (array[parent]->counter > tmp->counter) {
            array[hole] = array[parent];
            array[hole]->position = hole;
        }
        else {
            break;
        }
        hole = parent;
    }
    array[hole] = tmp;
    array[hole]->position = hole;
};

__device__ void perlocateDown(node* array[], int hole) {
    int child;
    node* tmp = array[hole];
    while (hole * 2 <= 1023) {
        child = hole * 2;
        if (child != 1023 && array[child + 1]->counter < array[child]->counter) {
            child++;
        }
        if (array[child]->counter < tmp->counter) {
            array[hole] = array[child];
            array[hole]->position = hole;
        }
        else {
            break;
        }
        hole = child;
    }
    array[hole] = tmp;
    array[hole]->position = hole;
};

__device__ void init_invert_page_table(VirtualMemory *vm) {
  
  for (int i = 0; i < vm->PAGE_ENTRIES; i++) {  // PAGE_ENTRIES = 1024, 4KB for each thread
    vm->invert_page_table[i] = 0x80000000 + i;   // valid bit: invalid: = 0x8000 0000, valid = 0x0, + virtual address
    vm->invert_page_table[i + vm->PAGE_ENTRIES] = 0x80000000 + i;
    vm->invert_page_table[i + 2*vm->PAGE_ENTRIES] = 0x80000000 + i;
    vm->invert_page_table[i + 3*vm->PAGE_ENTRIES] = 0x80000000 + i;

    node* new_node = new node;
    new_node->physical_address = i;
    new_node->position = i+1;
    vm->nodes[i] = new_node;
    vm->LRU[i+1] = new_node;
  }
}

__device__ void vm_init(VirtualMemory *vm, uchar *buffer, uchar *storage,
                        u32 *invert_page_table, int *pagefault_num_ptr,
                        int PAGESIZE, int INVERT_PAGE_TABLE_SIZE,
                        int PHYSICAL_MEM_SIZE, int STORAGE_SIZE,
                        int PAGE_ENTRIES) {
  // init variables
  vm->buffer = buffer;
  vm->storage = storage;
  vm->invert_page_table = invert_page_table;
  vm->pagefault_num_ptr = pagefault_num_ptr;

  // init constants
  vm->PAGESIZE = PAGESIZE;                                  // 32B
  vm->INVERT_PAGE_TABLE_SIZE = INVERT_PAGE_TABLE_SIZE;      // 16KB
  vm->PHYSICAL_MEM_SIZE = PHYSICAL_MEM_SIZE;                // 32KB
  vm->STORAGE_SIZE = STORAGE_SIZE;                          // 128KB
  vm->PAGE_ENTRIES = PAGE_ENTRIES;                          // 1024

  // before first vm_write or vm_read
  init_invert_page_table(vm);
}

__device__ uchar vm_read(VirtualMemory *vm, u32 addr) {
    int pid_adder = vm->pid * vm->PAGE_ENTRIES;
    vm->count++;
    u32 offset = addr - ((addr >> 5) << 5);
    addr = addr >> 5;
    for (int i = 0; i < 1024; i++) {    // if the data need to be read is in the buffer
        if (vm->invert_page_table[i + pid_adder] < 0x80000000 && vm->invert_page_table[i + pid_adder] == addr) {
            vm->nodes[i]->counter = vm->count;
            perlocateDown(vm->LRU, vm->nodes[i]->position);
            return vm->buffer[(i << 5) + offset];
        }
    }
    (*vm->pagefault_num_ptr)++;   // if the data need to be read is not in the buffer
    int address = vm->LRU[1]->physical_address;
    u32 vpn = vm->invert_page_table[address + pid_adder];
    for (int i = 0; i < 32; i++) {
        vm->storage[(vpn << 5) + i] = vm->buffer[(address << 5) + i];   // store page into the disk
        vm->buffer[(address << 5) + i] = vm->storage[(addr << 5) + i];  // load page from the disk
    }
    vm->invert_page_table[address + pid_adder] = addr;
    vm->LRU[1]->counter = vm->count;
    perlocateDown(vm->LRU, 1);
    return vm->buffer[(address << 5) + offset];

    /* Complate vm_read function to read single element from data buffer */
    //return 123; //TODO
}

__device__ void vm_write(VirtualMemory *vm, u32 addr, uchar value) {
    int pid_adder = vm->pid * vm->PAGE_ENTRIES;
    vm->count++;
    u32 offset = addr - ((addr >> 5) << 5);
    addr = addr >> 5;
    for (int i = 0; i < 1024; i++) {    // if the data need to be rewrite is in the buffer
        if (vm->invert_page_table[i + pid_adder] < 0x80000000 && vm->invert_page_table[i + pid_adder] == addr) {
            vm->buffer[(i << 5) + offset] = value;
            vm->nodes[i]->counter = vm->count;
            perlocateDown(vm->LRU, vm->nodes[i]->position);
            return;
        }
    }
    (*vm->pagefault_num_ptr)++;    // if it is a piece of new data or the data need to be rewrite is in the disk
    if (vm->pointer < 1024) {   // pointer point to the next physical address to be filled before physical memory filled
        vm->buffer[(vm->pointer << 5) + offset] = value;    // store the data
        vm->invert_page_table[vm->pointer + pid_adder] = addr; // set the valid bit and store the virtual address
        vm->nodes[vm->pointer]->counter = vm->count;    // update the LRU counter
        perlocateUp(vm->LRU, vm->nodes[vm->pointer]->position);
        perlocateDown(vm->LRU, vm->nodes[vm->pointer]->position);
        vm->pointer++;  // next position
    }
    else {  // swap data to disk according to LRU
        int address = vm->LRU[1]->physical_address;
        u32 vpn = vm->invert_page_table[address];
        for (int i = 0; i < 32; i++) {
            vm->storage[(vpn << 5) + i] = vm->buffer[(address << 5) + i];   // store page into the disk
            vm->buffer[(address << 5) + i] = vm->storage[(addr << 5) + i];  // load page from the disk
        }
        vm->buffer[(address << 5) + offset] = value;
        vm->invert_page_table[address + pid_adder] = addr;
        vm->LRU[1]->counter = vm->count;
        perlocateDown(vm->LRU, 1);
    }
  /* Complete vm_write function to write value into data buffer */
}

__device__ void vm_snapshot(VirtualMemory *vm, uchar *results, int offset,
                            int input_size) {
    for (int i = offset; i < input_size; i++)
        results[i] = vm_read(vm, i);
  /* Complete snapshot function togther with vm_read to load elements from data
   * to result buffer */
}

