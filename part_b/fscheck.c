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

	// DECLARATION
	void *img; // mmaped fs file
	// to point to the superblock struct (in fs.c)
	struct superblock *sblock;
	// to point to the dinode struct (in fs.c)
	struct dinode *di;
	// to point to the dirent struct (in fs.c)
	struct dirent *dir;
	// ref of stat struct (in stat.c)
	struct stat img_stat;
	char * bitmap; // bitmap array
	//int *bitmap[sblock->size]; // bitmap array
	int numBlocks; // number of data numBlocks
	//int bitmapSize;

	// INITIALIZATION
	int temp = fstat(fd, &img_stat); // getting file size
	// site: https://www.tutorialspoint.com/c_standard_library/c_macro_assert.htm
	assert(temp == 0); // If expression evaluates to TRUE, assert() does nothing.
	// If expression evaluates to FALSE, assert() displays an error
	// message on stderr
	// mmap usage points to the start of the file system/block 0
	img = mmap(NULL, img_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	assert(img != MAP_FAILED);
	//bitmapSize = (sblock->nblocks + sblock->ninodes / IPB + sblock->nblocks / BPB + 4) / 8;

	// since, superblock starts at index 1 (as block 0 + BSIZE: 512 bytes)
	sblock = (struct superblock*) (img + 1 * BSIZE);
	// since, inode table starts at index 2
	di = (struct dinode*) (BSIZE * 2 + img)) // points to first inode block
	// Since datablock starts at index 4,
	numBlocks = (sblock->ninodes * sizeof(struct dinode))/ BSIZE + 1;


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
				if ((di->type == T_DIR && i == 1) &&
					((dir + 1)->inum != 1 // checking the inum of the parent
					|| strcmp((dir + 1)->name, "..") != 0)) { // checking if .. dir exists
		    fprintf(stderr, "ERROR: root directory does not exist.\n");
				fflush(stderr);
		    exit(1);
		  }
			if (i == ROOTINO && di->type != T_DIR) {
				fprintf(stderr, "ERROR: root directory does not exist.\n");
				fflush(stderr);
		    exit(1);
			}
			di++;
		}
	}
	// check 2 : current dir mismatch ???
	// DIRECTORY CHECK END---------------------------------------------


	// INODE CHECKS---------------------------------------------
	// check 1 : bad inode
	for (int i = 0; i < sblock->ninodes; i++){
		if (di->type != T_DIR
			//&& di->type != T_SYM
	    && di->type != T_FILE
			&& di->type != T_DEV
			&& di->type != 0) {
	    fprintf(stderr, "ERROR: bad inode.\n");
			fflush(stderr);
	    exit(1);
		}
		di++;
}
	// check 2 : bad size ???
	// INODE CHECK END---------------------------------------------


	// BITMAP CHECKS-------------------------------------------
	// check 1 : bitmap marks data
	unit bitOffset = di->addrs[j]%8;
	uint blockOffset = di->addrs[j]/8;
	int flag = 0;
		if (di->type == T_DIR
			//&& di->type != T_SYM
			|| di->type == T_FILE
			|| di->type == T_DEV
			&& di->type != 0){
				flag = 1;
			}

	for(int i = 0; i < sblock->ninodes; i++){
		for (int j = 0; j < NDIRECT; j++){
			if (di->addrs[j] != 0 && flag == 1){
				// Since bitmap starts at index 3,
				bitmap = (char *)(BSIZE * BBLOCK(di->addrs[j], sb->ninodes) + img);
				// AND it with 1 to get value in the bitmap
				if ((((blockOffset + bitmap) >> bitOffset) & 1) != 1) {
					fprintf(stderr,"ERROR: bitmap marks data free but data block used by inode.\n");
					fflush(stderr);
					exit(1);
				}
			} else {
				continue;
			}
		}
	}
	// check 2 : bitmap marks data in use

	//version 1
	// for(int i = 0; i < sblock->ninodes; i++){
	// 	for (int j = 0; j < NDIRECT + 1; j++){
	// 	//if (IS_BLOCK_ALLOC(j, sblock, img)) { // ????
	// 		fprintf(stderr,"ERROR: bitmap marks data block in use but not used.\n");
	// 		fflush(stderr);
	// 		exit(1);
	// 	}
	// }


	//version2
	for(int i = 0; i < sblock->ninodes; i++){
		for (int j = 0; j < NDIRECT; j++){
			if (di->addrs[j] != 0 && flag == 1){
				// Since bitmap starts at index 3,
				bitmap = (char *)(BSIZE * BBLOCK(di->addrs[j], sb->ninodes) + img);
				// AND it with 1 to get value in the bitmap
				if ((((blockOffset + bitmap) >> bitOffset) & 1) != 0) {
					fprintf(stderr,"ERROR: bitmap marks data block in use but not used.\n");
					fflush(stderr);
					exit(1);
				}
			} else {
				continue;
			}
		}
	}

	// BITMAP CHECK END---------------------------------------------

	// FREE
    temp = munmap(img, img_stat.st_size);
    assert(temp == 0);

    int fd_close = close(fd);
    assert(fd_close == 0);

	exit(1);
}

