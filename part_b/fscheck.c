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
	// DECLARATION
	void *img;
	struct superblock *sblock;
	struct dinode *di;
	struct dirent *dirent;
	struct stat *stat;
	int bitmap[sblock->size]; // bitmap array
	int numBlocks; // number of data numBlocks


	// INITIALIZATION
	sblock = (img + BSIZE);

	if(argc < 2 || argc > 2){
		printf("illegal number of arguments");
		fflush();
	}
	int fd;
	fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    printf(stderr, "ERROR: image not found\n");
		fflush();
    exit(1);
  }

	numBlocks = (sblock->ninodes * sizeof(struct dinode))/ BSIZE + 1;
	// img = mmap(NULL, stat->size, , , fd, 0);
	sblock = (img + BSIZE);
	di = (BSIZE * 2 + img)) // inode table starts at index 2

	// Since bitmap starts at index 3,
	bitmap = (BSIZE * (sblock->ninodes/IPB + 3) + img);
	// Since datablock starts at index 4,
	// numBlocks = (BSIZE * (sblock->ninodes/IPB + .... + 4) + img);

	// CHECKS
	// DIRECTORY CHECKS
	// check 1 : root dir
  if ((ROOTINO + di)->type != T_DIR) {  //a) if type not directory
    printf(stderr, "ERROR: root directory does not exist.\n");
		fflush();
    exit(1);
  }
	// if (!(ROOTINO + di)){ // b) if it doesn't even exist
	// 	printf(stderr, "ERROR: root directory does not exist.\n");
	// 	fflush();
  //  exit(1);
	// }

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

