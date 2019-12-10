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

// SANITY CHECK - MAP
// block 0 = unused
// block 1: superblock
// structure :
// 0.unused --> 1.superblock -->2.inodetable --> 3.bitmap --> 4.data blocks

int main(int argc, char *argv[]) {
	// Check if number of arguments is correct.
	if(argc != 2){
		fprintf(stderr, "Illegal number of arguments.");
		fflush(stderr);
	}

	// File descriptor
	int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "ERROR: image not found.\n");
		fflush(stderr);
    exit(1);
  }

	// Get the size of the file
	struct stat st;
	fstat(fd, &st);

	void *img = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	struct superblock *sblock = img + BSIZE;
	// inode table starts at index 2
	struct dinode *di = BSIZE * 2 + img;
	// struct dinode *di = img + 1 * BSIZE;
	struct dirent *dirent;

	int bitmap[sblock->size];

	// Number of data blocks
	int num_blocks = (sblock->ninodes * sizeof(struct dinode))/ BSIZE + 1;

	// Since bitmap starts at index 3,
	bitmap = (BSIZE * (sblock->ninodes/IPB + 3) + img);
	// Since datablock starts at index 4,
	// num_blocks = (BSIZE * (sblock->ninodes/IPB + .... + 4) + img);

	// DIRECTORY CHECKS
	// check 1 : root dir
	for (int i = 0; i < sblock->ninodes; i++) {
	  if ((di->type != 1 && i == 1) ||
			(dirent + 1)->inum != 1) {  //a) if type not directory
	    fprintf(stderr, "ERROR: root directory does not exist.\n");
			fflush(stderr);
	    exit(1);
	  }
		di++;
	}
	// if (!(ROOTINO + di)){ // b) if it doesn't even exist
	// 	printf(stderr, "ERROR: root directory does not exist.\n");
	// 	fflush();
  //  exit(1);
	// }

	// check 2 : current dir mismatch
	// TODO

	// INODE CHECKS
	// check 1 : bad inode
	for (int i = 0; i < sblock->ninodes; i++){
		if (di->type != 1 // T_DIR
	    && di->type != 2 // T_FILE
			&& di->type != 3 // T_DEV
			&& di->type != 0) {
		    fprintf(stderr, "ERROR: bad inode.\n");
				fflush(stderr);
		    exit(1);
		}
}
	// check 2 : bad size
	// TODO

	// BITMAP CHECKS
	// check 1 : bitmap marks data free
	for (int i = 0; i < NDIRECT+1; i++){
		if (bitmap[di->addrs[i]] <= 0) {
			fprintf(stderr,"ERROR: bitmap marks data free but data block used by inode.\n");
			fflush(stderr);
			exit(1);
		}
	}

	// check 2 : bitmap marks data in use
	for (int i = 0; i < sblock->size; i++){
		if (i > num_blocks && bitmap[i] > 0) { // ????
			fprintf(stderr,"ERROR: bitmap marks data block in use but not used.\n");
			fflush(stderr);
			exit(1);
		}
	}

	// size of bitmap?
	// int size;
	// size = (sblock->nblocks + sblock->ninodes/IPB + sblock->nblocks/BPB + 4) / 8;



	exit(1);
}
