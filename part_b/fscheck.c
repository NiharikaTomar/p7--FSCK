#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#include "param.h"

//SANITY CHECK - MAP
// block 0 = unused
// block 1: superblock
// structure :
// 0.unused --> 1.superblock -->2.inodetable --> 3.bitmap --> 4.data blocks

int main(int argc, char *argv[]){
	// STD CHECKS
	if(argc < 2 || argc > 2){
		printf("illegal number of arguments");
		fflush();
	}
	// opening file
	int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    printf(stderr, "ERROR: image not found\n");
		fflush();
    exit(1);
  }

	// DECLARATION
	void *img;
	// to point to the superblock struct (in fs.c)
	struct superblock *sblock;
	// to point to the dinode struct (in fs.c)
	struct dinode *di;
	struct dirent *dir;
	struct stat img_stat;
	int *bitmap[sblock->size]; // bitmap array
	int numBlocks; // number of data numBlocks


	// INITIALIZATION
	int temp = fstat(fd, &img_stat); // getting file size

	// If expression evaluates to TRUE, assert() does nothing.
	// If expression evaluates to FALSE, assert() displays an error
	// message on stderr
	// site: https://www.tutorialspoint.com/c_standard_library/c_macro_assert.htm
	assert(temp == 0);
	// mmap usage points to the start of the file system/block 0
	img = mmap(NULL, img_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	assert(img != MAP_FAILED);



	numBlocks = (sblock->ninodes * sizeof(struct dinode))/ BSIZE + 1;
	// since, superblock starts at index 1 (as block 0 + BSIZE: 512 bytes)
	sblock = (struct superblock*) (img + 1 * BSIZE);
	// since, inode table starts at index 2
	di = (struct dinode*) (BSIZE * 2 + img)) // points to first inode block

	// Since bitmap starts at index 3,
	bitmap = (int *)(BSIZE * (sblock->ninodes/IPB + 3) + (int*)img);
	// Since datablock starts at index 4,
	// numBlocks = (BSIZE * (sblock->ninodes/IPB + .... + 4) + img);

	// CHECKS
	// DIRECTORY CHECKS
	// check 1 : root dir
	for (int i = 0; i < sblock->ninodes; i++) {
	  if ((di->type != T_DIR && i == 1)
				|| (dir + 1)->inum != 1) { // // checking if the inum of the parent
	    printf(stderr, "ERROR: root directory does not exist.\n");
			fflush();
	    exit(1);
	  }
		di++;
}
	// check 2 : current dir mismatch ???

	// INODE CHECKS
	// check 1 : bad inode
	for (int i = 0; i < sblock->ninodes; i++){
		if (di->type != T_DIR
			//&& di->type != T_SYM
	    && di->type != T_FILE
			&& di->type != T_DEV
			&& di->type != 0) {
	    printf(stderr, "ERROR: bad inode.\n");
			fflush();
	    exit(1);
		}
		//di++;
}
	// check 2 : bad size ???

	// BITMAP CHECKS
	// check 1 : bitmap marks data free
	for (int i = 0; i < NDIRECT+1; i++){
		if (bitmap[di->addrs[i]] <= 0)
		printf(stderr,"ERROR: bitmap marks data free but data block used by inode.\n");
		fflush();
		exit(1);
	}

	// check 2 : bitmap marks data in use
	for (int i = 0; i < sblock->size; i++){
		if (i > numBlocks && bitmap[i] > 0) { // ????
			printf(stderr,"ERROR: bitmap marks data block in use but not used.\n");
			fflush();
			exit(1);
		}
	}

	// size of bitmap?
	int size;
	// size = (sblock->nblocks + sblock->ninodes/IPB + sblock->nblocks/BPB + 4) / 8;



	exit(1);
}

