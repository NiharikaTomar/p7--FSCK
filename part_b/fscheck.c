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

void *img;
struct superblock *sblock;
struct dinode *di;
char bitmap[8];
void *datablock;


int main(int argc, char *argv[]){
	// sblock = (img + BSIZE);
	if(argc < 2 || argc > 2){
		printf("illegal number of arguments");
		fflush();
	}
	int fd;
	fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    printf(stderr, "ERROR: image not found\n");
		fflush();
    exit(1);
  }
	// img = mmap();
	sblock = (img + BSIZE);
	di = (BSIZE * 2 + img)) // inode table starts at index 2
	// block 0 = unused
	// block 1: superblock
 	// structure :
 	// 0.unused --> 1.superblock -->2.inodetable --> 3.bitmap --> 4.data blocks
	// Since bitmap starts at index 3,
	bitmap = (BSIZE * (sblock->ninodes/IPB + 3) + img);
	// Since datablock starts at index 4,
	// datablock = (BSIZE * (sblock->ninodes/IPB + .... + 4) + img);

	// CHECKS
	// DIRECTORY CHECKS
	// check 1 : root dir
  if ((ROOTINO + di)->type != T_DIR) {  //a)
    printf(stderr, "ERROR: root directory does not exist.\n");
		fflush();
    exit(1);
  }
	// if (!(ROOTINO + di)){ //b
	// 	printf(stderr, "ERROR: root directory does not exist.\n");
	// 	fflush();
  //   exit(1);
	// }

	// check 2 : current dir mismatch ???

	// INODE CHECKS
	// check 1 : bad inode
	if (di->type != T_DIR
		//&& di->type != T_SYM
    && di->type != T_FILE
		&& di->type != T_DEV) {
    printf(stderr, "ERROR: bad inode.\n");
		fflush();
    exit(1);
	}
	// check 2 : bad size ???

	// BITMAP CHECKS
	// check 1 : bitmap marks data free




	exit(1);
}

