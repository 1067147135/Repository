#include "file_system.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

__device__ __managed__ u32 gtime = 0;


__device__ void fs_init(FileSystem *fs, uchar *volume, int SUPERBLOCK_SIZE,
							int FCB_SIZE, int FCB_ENTRIES, int VOLUME_SIZE,
							int STORAGE_BLOCK_SIZE, int MAX_FILENAME_SIZE, 
							int MAX_FILE_NUM, int MAX_FILE_SIZE, int FILE_BASE_ADDRESS)
{
	// init variables
	fs->volume = volume;

	// init constants
	fs->SUPERBLOCK_SIZE = SUPERBLOCK_SIZE;		// 4096, 32K/8 bits = 4 K, bit wise free space management
	fs->FCB_SIZE = FCB_SIZE;					// 32, 32 bytes per FCB
	fs->FCB_ENTRIES = FCB_ENTRIES;				// 1024
	fs->STORAGE_SIZE = VOLUME_SIZE;				// 1085440, 4096+32768+1048576
	fs->STORAGE_BLOCK_SIZE = STORAGE_BLOCK_SIZE;// 32
	fs->MAX_FILENAME_SIZE = MAX_FILENAME_SIZE;	// 20
	fs->MAX_FILE_NUM = MAX_FILE_NUM;			// 1024
	fs->MAX_FILE_SIZE = MAX_FILE_SIZE;			// 1048576
	fs->FILE_BASE_ADDRESS = FILE_BASE_ADDRESS;	// 36864

	for (int i = 0; i < fs->STORAGE_SIZE; i++) {	// initialize as 0
		fs->volume[i] = 0;
	} 
}


__device__ void perlocateDown_D(FCB* array[], int hole, int size) { // first order: modified time; second order: create time
	int child;
	FCB* tmp = array[hole];
	while (hole * 2 <= size) {
		child = hole * 2;
		if (child != size && array[child + 1]->modify_time > array[child]->modify_time) {
			child++;
		}
		else if (child != size && array[child + 1]->modify_time == array[child]->modify_time) {
			if (array[child + 1]->create_time < array[child]->create_time) {
				child++;
			}
		}
		if (array[child]->modify_time > tmp->modify_time) {
			array[hole] = array[child];
		}
		else if (array[child]->modify_time == tmp->modify_time) {
			if (array[child]->create_time < array[hole]->create_time) {
				array[hole] = array[child];
			}
			else {
				break;
			}
		}
		else {
			break;
		}
		hole = child;
	}
	array[hole] = tmp;
};

__device__ void perlocateDown_S(FCB* array[], int hole, int size) { // first order: size; second order: create time
	int child;
	FCB* tmp = array[hole];
	while (hole * 2 <= size) {
		child = hole * 2;
		if (child != size && array[child + 1]->size > array[child]->size) {
			child++;
		}
		else if (child != size && array[child + 1]->size == array[child]->size) {
			if (array[child + 1]->create_time < array[child]->create_time) {
				child++;
			}
		}
		if (array[child]->size > tmp->size) {
			array[hole] = array[child];
		}
		else if (array[child]->size == tmp->size) {
			if (array[child]->create_time < array[hole]->create_time) {
				array[hole] = array[child];
			}
			else {
				break;
			}
		}
		else {
			break;
		}
		hole = child;
	}
	array[hole] = tmp;
};

__device__ void perlocateDown_C(FCB* array[], int hole, int size) { 
	int child;
	FCB* tmp = array[hole];
	while (hole * 2 <= size) {
		child = hole * 2;
		if (child != size && array[child + 1]->start_address < array[child]->start_address) {
			child++;
		}
		if (array[child]->start_address < tmp->start_address) {
			array[hole] = array[child];
		}
		else {
			break;
		}
		hole = child;
	}
	array[hole] = tmp;
};

__device__ uchar bit_change(uchar ch, int i, int v) { // change the ith bit in ch to v
	int bit[9];
	bit[8] = (ch >> 7);
	bit[7] = (ch >> 6) - (bit[8] << 1);
	bit[6] = (ch >> 5) - (bit[8] << 2) - (bit[7] << 1);
	bit[5] = (ch >> 4) - (bit[8] << 3) - (bit[7] << 2) - (bit[6] << 1);
	bit[4] = (ch >> 3) - (bit[8] << 4) - (bit[7] << 3) - (bit[6] << 2) - (bit[5] << 1);
	bit[3] = (ch >> 2) - (bit[8] << 5) - (bit[7] << 4) - (bit[6] << 3) - (bit[5] << 2) - (bit[4] << 1);
	bit[2] = (ch >> 1) - (bit[8] << 6) - (bit[7] << 5) - (bit[6] << 4) - (bit[5] << 3) - (bit[4] << 2) - (bit[3] << 1);
	bit[1] = ch - ((ch >> 1) << 1);

	bit[i] = v;
	uchar result = (bit[8] << 7) + (bit[7] << 6) + (bit[6] << 5) + (bit[5] << 4) + (bit[4] << 3) + (bit[3] << 2) + (bit[2] << 1) + bit[1];
	return result;	
}

__device__ int bit_check(uchar ch, int count) { // check continuous empty blocks(0)
	int bit[9];
	bit[8] = (ch >> 7);
	bit[7] = (ch >> 6) - (bit[8] << 1);
	bit[6] = (ch >> 5) - (bit[8] << 2) - (bit[7] << 1);
	bit[5] = (ch >> 4) - (bit[8] << 3) - (bit[7] << 2) - (bit[6] << 1);
	bit[4] = (ch >> 3) - (bit[8] << 4) - (bit[7] << 3) - (bit[6] << 2) - (bit[5] << 1);
	bit[3] = (ch >> 2) - (bit[8] << 5) - (bit[7] << 4) - (bit[6] << 3) - (bit[5] << 2) - (bit[4] << 1);
	bit[2] = (ch >> 1) - (bit[8] << 6) - (bit[7] << 5) - (bit[6] << 4) - (bit[5] << 3) - (bit[4] << 2) - (bit[3] << 1);
	bit[1] = ch - ((ch >> 1) << 1);

	for (int i = 1; i < 9; i++) {
		if (bit[i] == 0) {
			count++;
		}
		else {
			count = 0;
		}
	}
	
	return count;
}

__device__ int find_space(FileSystem* fs, int size = 32) {	// find a continuous (size = 1024/32) blocks memory 
	int count = 0;
	int start = 99999;	// if find, start = 0 ~ 32768; if not find, start = 99999
	int i;
	for (i = 0; i < fs->SUPERBLOCK_SIZE; i++) {	// 0 ~ 4096-1
		if (count >= size) {	// 1024/32 = 32 storage blocks
			start = i * 8 - count;
			break;
		}
		count = bit_check(fs->volume[i], count);
		//printf("count up to now: %d\n", count);
	}
	return start;
}

__device__ void compaction(FileSystem* fs) {
	FCB* min_heap[1025];	// start address low to high, move to lower empty blocks
	int count = 0;
	//printf("Let's do a full compaction!\n");
	for (int i = 0; i < fs->FCB_ENTRIES; i++) {
		int fcb = fs->SUPERBLOCK_SIZE + i * fs->STORAGE_BLOCK_SIZE;
		if (fs->volume[fcb] != 0) {	// not empty
			FCB* new_FCB = new FCB;
			new_FCB->index = i;
			new_FCB->start_address = fs->volume[fcb + 20] + (fs->volume[fcb + 21] << 8);
			min_heap[1 + count++] = new_FCB;
		}
	}
	//printf("Process: filled min heap!\n");
	for (int i = count / 2; i > 0; i--) {	// build the min heap
		perlocateDown_C(min_heap, i, count);
	}
	//printf("Process: built min heap!\n");
	for (int j = count; j > 0; j--) {
		//printf("Process: start %d\n", j);
		int index = min_heap[1]->index;
		int address = min_heap[1]->start_address;	// block index
		int fcb = fs->SUPERBLOCK_SIZE + index * fs->STORAGE_BLOCK_SIZE;
		int size = fs->volume[fcb + 22] + (fs->volume[fcb + 23] << 8);
		int fp1 = fs->FILE_BASE_ADDRESS + address * fs->STORAGE_BLOCK_SIZE;	// old data address
		//printf("Process: identify old address of %d: %d\n", index, address);
		for (int i = 0; i < ((size + 31) / 32); i++) {	// clear free space management
			int byte = (address + i) / 8;
			int offset = (address + i) % 8 + 1;
			//printf("%d->", fs->volume[byte]);
			fs->volume[byte] = bit_change(fs->volume[byte], offset, 0);
			//printf("%d\n", fs->volume[byte]);
		}
		//printf("Process: cleared free space management, size:%d\n", ((size + 31) / 32));
		int new_address = find_space(fs, ((size + 31) / 32));
		//printf("Process: find new address: %d\n", new_address);
		if (new_address == 99999) {
			for (int i = 0; i < ((size + 31) / 32); i++) {	// modify back free space management
				int byte = (address + i) / 8;
				int offset = (address + i) % 8 + 1;
				fs->volume[byte] = bit_change(fs->volume[byte], offset, 1);
			}
			min_heap[1]->start_address = INT_MAX - 10;	// push it to the end
			perlocateDown_C(min_heap, 1, count);
			//printf("Process: skiped %d\n", j);
			continue;
		}
		int fp2 = fs->FILE_BASE_ADDRESS + new_address * fs->STORAGE_BLOCK_SIZE;	// new data address
		for (u32 i = 0; i < size; i++) {				// write in file data
			fs->volume[fp2 + i] = fs->volume[fp1 + i];
		}
		//printf("Process: write the file\n");
		for (int i = 0; i < ((size + 31) / 32); i++) {	// fill free space management
			int byte = (new_address + i) / 8;
			int offset = (new_address + i) % 8 + 1;
			fs->volume[byte] = bit_change(fs->volume[byte], offset, 1);
		}
		//printf("Process: filled free space management\n");
		fs->volume[fcb + 20] = new_address - ((new_address >> 8) << 8);	// update new start address in FCB
		fs->volume[fcb + 21] = new_address >> 8;
		min_heap[1]->start_address = INT_MAX - 10;	// push it to the end
		//printf("Process: stored new address\n");
		perlocateDown_C(min_heap, 1, count);
		//printf("Process: moved %d\n", j);
	}
	//printf("Full compaction finished!\n");
}



__device__ u32 fs_open(FileSystem *fs, char *s, int op)
{
	gtime++;
	// Check if it exist in FCB
	int fcb;	// fcb address
	int empty = 0;	// next empty fcb
	bool flag = false;	// find it or not
	u32 address;	// address to store the data = FCB[20:23] 
	for (int i = 0; i < fs->FCB_ENTRIES; i++) {	// try to find exist fcb
		fcb = fs->SUPERBLOCK_SIZE + i * fs->STORAGE_BLOCK_SIZE;
		int qointer = fcb;
		int pointer = 0;
		if (fs->volume[fcb] == 0 && empty == 0) {
			empty = fcb;
		}
		while (fs->volume[qointer] != 0 || s[pointer] != 0) {	// check the file name
			if (fs->volume[qointer] == s[pointer]) {
				//printf("i = %d: %c<->%c\n", i, fs->volume[qointer], s[pointer]);
				qointer++;
				pointer++;
				flag = true;
			}
			else {
				flag = false;
				break;
			}
		}
		if (flag == true) {
			break;
		}
	}
	if (flag == true) {	// find the file
		//printf("find the exist file at %d\n",(fcb - fs->SUPERBLOCK_SIZE)/fs->STORAGE_BLOCK_SIZE);
		address = fs->volume[fcb + 20] + (fs->volume[fcb + 21] << 8);	// 0 ~ 32768-1
		if (op == G_WRITE) {	// op == G_WRITE, need clear; op == G_READ, only need to return the address
			int size = (fs->volume[fcb + 22] + (fs->volume[fcb + 23] << 8) + 31) / 32;	// unit: storage block, 0~32
			for (int i = 0; i < size; i++) {	// clear free space management may be enough
				int byte = (address + i) / 8;
				int offset = (address + i) % 8 + 1;
				fs->volume[byte] = bit_change(fs->volume[byte], offset, 0);
			}
			address = find_space(fs);	// find another space for new write in content
			if (address >= 99999) {	// try full compaction once
				compaction(fs);
				address = find_space(fs);
			}
			if (address < 99999) {
				fs->volume[fcb + 20] = address - ((address >> 8) << 8);	// update new start address in FCB
				fs->volume[fcb + 21] = address >> 8;
				fs->volume[fcb + 22] = 0;	// clear the size in FCB
				fs->volume[fcb + 23] = 0;
				fs->volume[fcb + 28] = gtime - ((gtime >> 8) << 8);	// update modify time
				fs->volume[fcb + 29] = (gtime >> 8) - ((gtime >> 16) << 8);
				fs->volume[fcb + 30] = (gtime >> 16) - ((gtime >> 24) << 8);
				fs->volume[fcb + 31] = gtime >> 24;
				//printf("new address: %d\n", fs->volume[fcb + 20] + (fs-find_space>volume[fcb + 21] << 8));
			}
			else {
				printf("Attention: Memory is completely full!!\n");
			}
		}
	}
	else {	// New file, find a contiguous 1024 bytes of block of memory, modify the FCB
		//printf("create a new file at %d\n", (empty - fs->SUPERBLOCK_SIZE) / fs->STORAGE_BLOCK_SIZE);
		address = find_space(fs);
		if (address >= 99999) {	// try full compaction once
			compaction(fs);
			address = find_space(fs);
		}
		fcb = empty;
		if (address >= 99999) {
			printf("Attention: Memory is completely full!!\n");
		}
		else if (empty == 0) {
			printf("Attention: FCB entries is completely full!!\n");
		}
		else {
			if (op == G_READ) {
				printf("Attention: No such file to read!!\n");
			}
			else {
				int qointer = empty;	// create a new FCB
				int pointer = 0;
				while (s[pointer] != 0) {	// fill the file name in FCB
					fs->volume[qointer] = s[pointer];
					qointer++;
					pointer++;
				}
				fs->volume[qointer] = '\0';
				fs->volume[empty + 20] = address - ((address >> 8) << 8);	// update new start address in FCB
				fs->volume[empty + 21] = address >> 8;
				fs->volume[empty + 22] = 0;	// clear the size (bytes) in FCB
				fs->volume[empty + 23] = 0;
				fs->volume[empty + 24] = gtime - ((gtime >> 8) << 8);	// update create time
				fs->volume[empty + 25] = (gtime >> 8) - ((gtime >> 16) << 8);
				fs->volume[empty + 26] = (gtime >> 16) - ((gtime >> 24) << 8);
				fs->volume[empty + 27] = gtime >> 24;
				fs->volume[empty + 28] = gtime - ((gtime >> 8) << 8);	// update modify time
				fs->volume[empty + 29] = (gtime >> 8) - ((gtime >> 16) << 8);
				fs->volume[empty + 30] = (gtime >> 16) - ((gtime >> 24) << 8);
				fs->volume[empty + 31] = gtime >> 24;
			}
		}
	}
	//printf("open file at %d\n", address);
	return fcb; // return the address of fcb
}


__device__ void fs_read(FileSystem *fs, uchar *output, u32 size, u32 fp)
{
	
	gtime++;
	int fcb = fp;
	int address = fs->volume[fcb + 20] + (fs->volume[fcb + 21] << 8);
	fp = fs->FILE_BASE_ADDRESS + address * fs->STORAGE_BLOCK_SIZE;
	u32 f_size = fs->volume[fcb + 22] + (fs->volume[fcb + 23] << 8);
	if (f_size < size) {
		printf("Attention: read out of bounds, only read the contents of the current file!!\n");
		size = f_size;
	}
	for (u32 i = 0; i < size; i++) {
		output[i] = fs->volume[fp+i];
	}
	//printf("read file at %d\n", address);
}

__device__ u32 fs_write(FileSystem *fs, uchar* input, u32 size, u32 fp)
{
	gtime++;
	int fcb = fp;
	int address = fs->volume[fcb + 20] + (fs->volume[fcb + 21] << 8);
	fp = fs->FILE_BASE_ADDRESS + address * fs->STORAGE_BLOCK_SIZE;
	for (u32 i = 0; i < size; i++) {				// write in file data
		fs->volume[fp + i] = input[i];
	}
	for (u32 i = 0; i < ((size + 31) / 32); i++) {	// update Free Space Management
		int byte = (address + i) / 8;
		int offset = (address + i) % 8 + 1;
		fs->volume[byte] = bit_change(fs->volume[byte], offset, 1);
	}
	fs->volume[fcb + 22] = size - ((size >> 8) << 8);	// update size
	fs->volume[fcb + 23] = size >> 8;
	fs->volume[fcb + 28] = gtime - ((gtime >> 8) << 8);	// update modify time
	fs->volume[fcb + 29] = (gtime >> 8) - ((gtime >> 16) << 8);
	fs->volume[fcb + 30] = (gtime >> 16) - ((gtime >> 24) << 8);
	fs->volume[fcb + 31] = gtime >> 24;
	
	//printf("write file at %d\n", address);
	return 0;
	/* Implement write operation here */
}

__device__ void fs_gsys(FileSystem *fs, int op)	// LS_D: order by modified time/ LS_S: list with size, order by size
{
	gtime++;
	FCB* min_heap[1025];
	int count = 0;
	if (op == LS_D) {
		printf("==sort by modified time==\n");
		for (int i = 0; i < fs->FCB_ENTRIES; i++) {
			int fcb = fs->SUPERBLOCK_SIZE + i * fs->STORAGE_BLOCK_SIZE;
			if (fs->volume[fcb] != 0) {
				FCB* new_FCB = new FCB;
				new_FCB->index = i;
				new_FCB->modify_time = fs->volume[fcb + 28] + (fs->volume[fcb + 29] << 8) + (fs->volume[fcb + 30] << 16) + (fs->volume[fcb + 31] << 24);
				new_FCB->create_time = fs->volume[fcb + 24] + (fs->volume[fcb + 25] << 8) + (fs->volume[fcb + 26] << 16) + (fs->volume[fcb + 27] << 24);
				min_heap[1 + count++] = new_FCB;
			}
		}
		for (int i = count / 2; i > 0; i--) {	// build the min heap
			perlocateDown_D(min_heap, i, count);
		}
		for (int i = count; i > 0; i--) {
			int qointer = fs->SUPERBLOCK_SIZE + min_heap[1]->index * fs->STORAGE_BLOCK_SIZE;
			char name[20];
			int j = 0;
			for (j = 0; j < 20; j++) {
				char ch = fs->volume[qointer + j];
				name[j] = ch;
				if (ch == 0) break;
			}
			for (; j < 20; j++) {
				name[j] = 0;
			}
			printf("%s\n", name);
			//while (fs->volume[qointer] != '\0') {	
			//	printf("%c", fs->volume[qointer++]);
			//}
			//printf("\n");
			min_heap[1]->modify_time = -1;	// push it to the end
			perlocateDown_D(min_heap, 1, count);
		}
	}
	else if (op == LS_S) {
		printf("==sort by file size==\n");
		for (int i = 0; i < fs->FCB_ENTRIES; i++) {
			int fcb = fs->SUPERBLOCK_SIZE + i * fs->STORAGE_BLOCK_SIZE;
			if (fs->volume[fcb] != 0) {
				FCB* new_FCB = new FCB;
				new_FCB->index = i;
				new_FCB->size = fs->volume[fcb + 22] + (fs->volume[fcb + 23] << 8);
				//printf("count = %d, size = %d\n", count, new_FCB->size);
				new_FCB->create_time = fs->volume[fcb + 24] + (fs->volume[fcb + 25] << 8) + (fs->volume[fcb + 26] << 16) + (fs->volume[fcb + 27] << 24);
				min_heap[1 + count++] = new_FCB;
			}

		}
		for (int i = count / 2; i > 0; i--) {	// build the min heap
			perlocateDown_S(min_heap, i, count);
		}
		for (int i = count; i > 0; i--) {
			int qointer = fs->SUPERBLOCK_SIZE + min_heap[1]->index * fs->STORAGE_BLOCK_SIZE;
			char name[20];
			int j = 0;
			for (j = 0; j < 20; j++) {
				char ch = fs->volume[qointer + j];
				name[j] = ch;
				if (ch == 0) break;
			}
			for (; j < 20; j++) {
				name[j] = 0;
			}
			printf("%s\t%d\n", name, min_heap[1]->size);
			//while (fs->volume[qointer] != 0) {	// check the file name
			//	printf("%c", fs->volume[qointer++]);
			//}
			//printf("\t%d\n", min_heap[1]->size);
			min_heap[1]->size = -1;	// push it to the end
			perlocateDown_S(min_heap, 1, count);
		}
	}
	
	/* Implement LS_D and LS_S operation here */
}

__device__ void fs_gsys(FileSystem *fs, int op, char *s)	// RM
{
	gtime++;
	int fcb;	// fcb address
	bool flag = false;	// find it or not
	for (int j = 0; j < fs->FCB_ENTRIES; j++) {	// try to find exist fcb
		fcb = fs->SUPERBLOCK_SIZE + j * fs->STORAGE_BLOCK_SIZE;
		int qointer = fcb;
		int pointer = 0;
		while (fs->volume[qointer] != 0 || s[pointer] != 0) {	// check the file name
			if (fs->volume[qointer] == s[pointer]) {
				qointer++;
				pointer++;
				flag = true;
			}
			else {
				flag = false;
				break;
			}
		}
		if (flag == true) {
			fs->volume[fcb] = 0;
			u32 address = fs->volume[fcb + 20] + (fs->volume[fcb + 21] << 8);
			u32 size = fs->volume[fcb + 22] + (fs->volume[fcb + 23] << 8);
			for (u32 i = 0; i < ((size + 31) / 32); i++) {	// update Free Space Management
				int byte = (address + i) / 8;
				int offset = (address + i) % 8 + 1;
				fs->volume[byte] = bit_change(fs->volume[byte], offset, 0);
			}
			break;
		}
	}
	if (flag == false) {
		printf("Attention: fail to find the file to remove");
	}
	/* Implement rm operation here */
}
