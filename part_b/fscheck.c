#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fs.h"
// #include "stat.h"

// #define IS_BLOCK_ALLOC(i, sblock, img) ((*((char*)img + (sblock->bmapstart)*BSIZE + (i/8))) & (1 << (i%8)))

// SANITY CHECK - MAP
// block 0 = unused
// block 1: superblock
// structure :
// 0.unused --> 1.superblock -->2.inodetable --> 3.bitmap --> 4.data blocks

int main(int argc, char *argv[]) {
	// Check if number of arguments is correct.
	if(argc != 2){
		fprintf(stderr, "Usage: fscheck file_system_image\n.");
		fflush(stderr);
	}

	// File descriptor
	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "ERROR: image not found.\n");
		fflush(stderr);
		exit(1);
	}

	void *img; // mmaped fs file
	// ref of stat struct (in stat.c)
	struct stat img_stat;
	int temp = fstat(fd, &img_stat); // getting file size
	// site: https://www.tutorialspoint.com/c_standard_library/c_macro_assert.htm
	assert(temp == 0); // If expression evaluates to TRUE, assert() does nothing.
	// If expression evaluates to FALSE, assert() displays an error
	// message on stderr	
	img = mmap(NULL, img_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	// mmap usage points to the start of the file system/block 0	
	assert(img != MAP_FAILED);
	// to point to the superblock struct (in fs.c)
	struct superblock *sblock;
	// since, superblock starts at index 1 (as block 0 + BSIZE: 512 bytes)
	sblock = (struct superblock*) (img + 1 * BSIZE);
	// to point to the dinode struct (in fs.c)
	struct dinode *di;
	// to point to the dirent struct (in fs.c)
	struct dirent *dir;
	// given bitmap
	char *bmap;
	// our bitmap
	uint *bitmap = malloc(sblock->size * sizeof(uint));
	// Since bitmap starts at index 3,
	bmap = (char *)(BSIZE * sblock->bmapstart + img);
	// since, inode table starts at index 2
	di = (struct dinode*) (img + (BSIZE * sblock->inodestart)); // points to first inode block
	// Dirent
	dir = (struct dirent*) (img + (BSIZE * di->addrs[0]));

	// Get address
	for (int i = 0; i < sblock->size; i+=8) {
		for (int j = 0; j < 8; j++) {
			bitmap[i+j] = (*bmap >> j) &0x1;
		}
		bmap++;
	}

	// uint inodes[sblock->size];
	// int index = 0;
	// for (index = 0; index < (sblock->size - sblock->nblocks); index++){
	// 	inodes[index] = 1;
	// }
	// int direct_used = 0;
	// for (int i = 0; i < sblock->ninodes; i++) {
	// 	if (dir->inum != 0) {
	// 		direct_used = 1;
	// 	}
	// }


	// CHECKS------------------------------------------------------------

	// Multi-Structure Checks-----------------------------------------------

	// Multi-Structure CHECK END---------------------------------------------


	// DIRECTORY CHECKS-----------------------------------------------
	// check 1 : root dir
	for (int i = 0; i < sblock->ninodes; i++) {
		// If we reach a free inode, move onto the next inode
		if(!di->type){
			// move onto the next inode
			di++;
			continue;
		} else {
			if ((di->type == 1 && i == ROOTINO) &&
				((dir[1]).inum != 1 // checking the inum of the parent
				|| strcmp((dir[1]).name, "..") != 0)) { // checking if .. dir exists
			    	fprintf(stderr, "ERROR: root directory does not exist.\n");
					fflush(stderr);
			    	exit(1);
		  	}
			if ((di->type == 1 && i == ROOTINO) &&
				((dir[0]).inum != 1 // checking the inum of the parent
				|| strcmp((dir[0]).name, ".") != 0)) { // checking if . dir exists
			    	fprintf(stderr, "ERROR: root directory does not exist.\n");
					fflush(stderr);
			    	exit(1);
		  	}
			di++;
		}
	}
	// check 2 : current dir mismatch ???
	// DIRECTORY CHECK END---------------------------------------------


	// INODE CHECK 1---------------------------------------------
	// check 1 : bad inode
	for (int i = 0; i < sblock->ninodes; i++){
		if (di->type != 1
	    	&& di->type != 2
			&& di->type != 3
			&& di->type != 0) {
	    		fprintf(stderr, "ERROR: bad inode.\n");
				fflush(stderr);
	   			exit(1);
		}
		di++;
	}
	// INODE CHECK 1 END---------------------------------------------


	// BITMAP CHECKS-------------------------------------------
	// check 1 : bitmap marks data
	for(int i = 0; i < sblock->size; i++){
		int flag = 0;
		if ((di->type == 1
			//&& di->type != T_SYM
			|| di->type == 2
			|| di->type == 3)
			&& di->type != 0){
				flag = 1;
		}
		// if (di->addrs[j] != 0 && flag == 1){
		if (flag == 1){

			// AND it with 1 to get value in the bitmap
			if (bitmap[di->addrs[i]] != 1) {
				fprintf(stderr,"ERROR: bitmap marks data free but data block used by inode.\n");
				fflush(stderr);
				exit(1);
			}
		} else {
			continue;
		}
	
	}
	// check 2 : bitmap marks data in use
	for(int i = 0; i < sblock->size; i++){
		int flag = 0;
		if ((di->type == 1
			//&& di->type != T_SYM
			|| di->type == 2
			|| di->type == 3)
			&& di->type != 0){
				flag = 1;
		}
		// if (di->addrs[j] != 0 && flag == 1){
		if (flag == 1){
			// AND it with 1 to get value in the bitmap
			if (bitmap[di->addrs[i]] != 0) {
				fprintf(stderr,"ERROR: bitmap marks data block in use but not used.\n");
				fflush(stderr);
				exit(1);
			}
		} else {
			continue;
		}
	}

	// BITMAP CHECK END---------------------------------------------

	



	// INODE CHECK 2---------------------------------------------
	// check 2 : bad size
	int count = 0;
	int max = 0;
	int min = 0;
	// uint *ind;
	for(int i = 0; i < sblock->ninodes; i++) {
		int flag = 0;
		if ((di->type == 1
			|| di->type == 2
			|| di->type == 3)
			&& di->type != 0){
				flag = 1;
		}
		if (flag == 1) {
			for (int j = 0; j < NINDIRECT; j++) {
				// ind = (uint *) ((j * sizeof(uint)) + (di->addrs[NDIRECT] * BSIZE) + img);
				// if (((ind * BSIZE) + img) != 0) {
				// 	count++;
				// }
			}
			min = (count - 1) * BSIZE;
			max = count * BSIZE; 
			if (((di->addrs[NDIRECT] * BSIZE) + img) != 0) {
				if (min < di->size)	{
					if (max > di->size) {
						fprintf(stderr, "ERROR: bad size in inode.\n");
						fflush(stderr);
						exit(1);
					}
				}
				dir++;
			}
			// ind++;
		}
		di++;
	}

	// INODE CHECK 2 END---------------------------------------------



	// FREE
    temp = munmap(img, img_stat.st_size);
    assert(temp == 0);

    int fd_close = close(fd);
    assert(fd_close == 0);

	exit(1);
}