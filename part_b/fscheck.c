// Copyright 2019 Niharika Tomar Elizaveta Stepanova
////////////////////////////////////////////////////////////////////////////////
// Main File:        fscheck.c
// Other Files:      fs.h, stat.h
// Semester:         CS 537 FALL 2019
//
// Author:           Niharika Tomar
// Email:            ntomar@wisc.edu
// CS Login:         tomar
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   tutorialspoint.com for macros def.
//                   stackoverflow.com for getting file size
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fs.h"

// macros
#define assert(cond, msg)  if (!(cond)) { fprintf(stderr, msg); exit(1); }

// SANITY CHECK - MAP
// block 0 = unused
// block 1: superblock
// structure :
// 0.unused --> 1.superblock -->2.inodetable --> 3.bitmap --> 4.data blocks

int main(int argc, char *argv[]) {
  // Check if number of arguments is correct.
  if (argc != 2) {
    printf("Usage: fscheck file_system_image\n.");
    exit(1);
  }

  // open file
  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
  printf("ERROR: image not found.\n");
    exit(1);
  }

  // getting file size
  struct stat buf;
  fstat(fd, &buf);
  off_t size = buf.st_size;

  void *img;  // mmaped fs file
  img = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  // mmap usage points to the start of the file system/block 0

  // to point to the superblock struct (in fs.c)
  struct superblock *sblock = (struct superblock*) (img + (1 * BSIZE));
  // since, superblock starts at index 1 (as block 0 + BSIZE: 512 bytes)

  // to point to the dinode struct (in fs.c)
  struct dinode *di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  // points to first inode block

  // to point to the dirent struct (in fs.c)
  struct dirent *dir = (struct dirent * )(img + BSIZE * di->addrs[0]);

  // given bitmap for calculations
  char *bitmap2 = (char *)(BSIZE * sblock->bmapstart + img);
  // our bitmap
  uint *bitmap = malloc(sblock->size * sizeof(uint));

  // CHECKS------------------------------------------------------------

  // BITMAP CHECKS-------------------------------------------

  // inodes array initialisation
  int size_block = sblock->size;
  // inode array for own calculations
  uint *inodes = malloc(1024 * sizeof(uint));
  int limit = size_block - sblock->nblocks;
  int lim = 0;
  while (lim < limit) {
    // initilaising all indices of inode array to 1
    inodes[lim] = 1;
    lim++;
  }
  // bitmap initialisation
  int upper_bound = 8;  // block has 8 bytes
  for (int i = 0; i < size_block; i+=upper_bound) {
    for (int j = 0; j < upper_bound; j++) {
      // shift and bitwise AND for later comparison
      bitmap[i+j] = (* bitmap2 >> j) & 0x1;
    }
    bitmap2++;
  }

  // check 1 : bitmap marks data free
  // reinitialise dinode struct
  di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  // int bflag = 0;
  // if(di->type == 0){  // free inode
  // bflag = 1;
  // }
  int a = 0;
  while (a < sblock->ninodes) {
    uint * ind;
    // for indirect blocks
    int indirect = 0;
    if (((di->addrs[NDIRECT] * BSIZE) + img) != 0) {
      while (indirect < NINDIRECT) {
        ind = (uint *)((indirect * sizeof(uint))
        + (di->addrs[NDIRECT] * BSIZE) + img);
        assert((!(*ind != 0 && bitmap[ * ind] == 0)),
         "ERROR: bitmap marks data free but data block used by inode.\n");
        indirect++;
      }
    }
    // for direct blocks
    int direct = 0;
    while (direct < NDIRECT) {
      assert((!(di->addrs[direct] != 0 && bitmap[di->addrs[direct]] == 0)),
       "ERROR: bitmap marks data free but data block used by inode.\n");
      direct++;
    }
    di++;
    a++;
  }

  // check 2 : bitmap marks data in use
  // reinitialise dinode struct
  di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  // di = (struct dinode * )(img + (2 * BSIZE) + (sblock->nlog * BSIZE));
  // uint *inodes = malloc(sblock->size * sizeof(uint));
  // // int index = 0;
  // // initilaising all indices of inode array to 1
  // for (int i = 0; i < (sblock->size - sblock->nblocks); i++){
  // inodes[i] = 1;
  // }
  int f = 0;
  while (f < sblock->ninodes) {
    // if (di->addrs[j] != 0 && flag == 1){
    if (di->type != 0) {
    // if (bflag != 1){ // if inode is not free
      // for direct blocks
      int dir_num = 0;
      while (dir_num <= NDIRECT) {
        if (di->addrs[dir_num] == 0) {
          break;
        } else {
          // update inode array
          inodes[di->addrs[dir_num]] = 1;
          // fprintf(stderr,"ERROR: bitmap marks data block in use
          // but not used.\n");
          // fflush(stderr);
          // exit(1);
        }
        dir_num++;
      }
      // for indirect blocks
      if (((di->addrs[NDIRECT] * BSIZE) + img) != 0) {
        uint * ind;
        int p = 0;
        while (p < NINDIRECT) {
          ind = (uint *)((p * sizeof(uint)) +
          (di->addrs[NDIRECT] * BSIZE) + img);
          if (*ind == 0) {
            break;
          } else {
            // update inode array
            inodes[* ind] = 1;
            // check 1
            // uint * msind_block;
          }
          p++;
        }
      }
    }
    di++;
    assert((!(bitmap[f] == 1 && inodes[f] == 0)),
     "ERROR: bitmap marks data block in use but not used.\n");
    f++;
  }
  // di++;
  // }

// BITMAP CHECK END---------------------------------------------

// DIRECTORY CHECKS-----------------------------------------------
// version 1 -----------------------------------------------------------
// if ((di->type == 1  && dir[1].inum != 1) // checking the inum of the parent
// || (!(strcmp(dir[1].name, "..")))) { // checking if .. dir exists
// fprintf(stderr, "ERROR: root directory does not exist.\n");
// fflush(stderr);
// exit(1);
// }
// if ((di->type == 1  && dir[0].inum != 1) // checking the inum of the parent
// || (!(strcmp((dir[0]).name, ".")))) { // checking if . dir exists
// fprintf(stderr, "ERROR: root directory does not exist.\n");
// fflush(stderr);
// exit(1);
// }
// --------------------------------------------------------------------

  // reinitialise dinode struct
  di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  // int dflag = 0;
  // if(di->type == 1){  // free inode
  // dflag = 1;
  // }
  int cur = 0;
  while (cur <= NDIRECT) {
    if (di->type == 1) {
      // reinitialise dirent struct
      dir = (struct dirent * )(img + BSIZE * di->addrs[0]);
      assert(!(dir->inum != 1),
       "ERROR: current directory mismatch.\n");
    }
    di++;
    cur++;
  }
  // reinitialise dinode struct
  di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  // traverse di
  di++;
  assert(!(di->type ==1 && dir->inum != 1),
  "ERROR: root directory does not exist.\n");
  dir++;
  assert(!(di->type ==1 && dir->inum != 1),
  "ERROR: root directory does not exist.\n");
  // DIRECTORY CHECK END---------------------------------------------

  // MULTI-STRUCTURE CHECK-----------------------------------------------
  // check 1
  // int msflag1 = 0;
  // int msflag2 = 0;
  // int msflag3 = 0;
  // int flagInUse = 0;
  // int flagDirInUse = 0;
  // uint * msind_block;
  // msind_block = (uint * )(img +(di->addrs[NDIRECT] * BSIZE));

  int numBlocks = BSIZE / sizeof(struct dirent);
  // inode in use
  uint *inUse = malloc(sblock->ninodes * sizeof(uint));
  // for inum in own copy of dir
  uint *dirinUse = calloc(sblock->ninodes, sizeof(uint));
  // reinitialise dinode struct
  di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  int in = 0;
  while (in < sblock->ninodes) {
    if (di->type == 0) {
      inUse[in] = 0;
    } else {
      inUse[in] = 1;
    }
    di++;
    in++;
    }

  // reinitialise dinode struct
  di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  int in2 = 0;
  while (in2 < sblock->ninodes) {
    // int cond = (dir->inum != 0) ? 1 : 0;
    // int type = (di->type == 1) ? 1 : 0;
    // if (msflag2 == 1) {
    if (di->type == 1) {  // if type dir
      // for direct blocks
      int d = 0;
      while (d < NDIRECT) {
        // reinitialise dirent struct
        dir = (struct dirent *)(img + BSIZE * di->addrs[d]);
        for (int k = 0; k < numBlocks; k++) {
          if (dir->inum != 0) {
            // msflag3 = 1;
            dirinUse[dir->inum] = 1;
          }
          // dirinUse[dir->inum] = (msflag3 == 0) ? 1 : 0
          dir++;
          // if (inUse[i] == 1) {
          // flagInUse = 1;
          // }
          // if (dirinUse[i] == 1) {
          // flagDirInUse = 1;
          // }
        }
        d++;
      }
      // for indirect blocks
      uint * ind = (uint *)((di->addrs[NDIRECT] * BSIZE) + img);
      int l = 0;
      while (l < NINDIRECT) {
        // reinitialise dirent struct
        dir = (struct dirent *) img + ((*ind) * BSIZE);
        int m = 0;
        while (m < numBlocks) {
          if (dir->inum != 0) {
            // msflag3 = 1;
            dirinUse[dir->inum] = 1;
          }
          // dirinUse[dir->inum] = (msflag3 == 0) ? 1 : 0;
          dir++;
          // if (inUse[i] == 1) {
          // flagInUse = 1;
          // }
          // if (dirinUse[i] == 1) {
          // flagDirInUse = 1;
          // }
          m++;
        }
        ind++;
        l++;
      }
    }
    di++;
    in2++;
  }
  int n = 0;
  while (n < sblock->ninodes) {
    int check = (inUse[n] == 1) ? 1 : 0;
    int check2 = (dirinUse[n] == 1) ? 1 : 0;
    assert((!(check == 1 && check2 == 0)),
     "ERROR: inode marked in use but not found in a directory.\n");
    assert((!(check == 0 && check2 == 1)),
     "ERROR: inode marked free but referred to in directory.\n");
    n++;
  }
  // Multi-Structure CHECK END---------------------------------------------

  // INODE CHECK 1---------------------------------------------
  // reinitialise dinode struct
  di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  // check 1 : bad inode
  // int iflag = 2;
  // // if (di->type != 1
  // // && di->type != T_SYM
  // // || di->type != 2
  // // || di->type != 3){
  // // iflag = 1;
  // // }
  int iter = 0;
  while (iter < sblock->ninodes) {
    assert((!(di->type > 3 || di->type < 0)), "ERROR: bad inode.\n");
    di++;
    iter++;
  }
  // INODE CHECK 1 END---------------------------------------------

  // INODE CHECK 2---------------------------------------------
  // check 2 : bad size
  di = (struct dinode*) (img + (BSIZE * sblock->inodestart));
  int cnt = 0;
  while (cnt < sblock->ninodes) {
    int inc = 0;
    uint *ind;
    // int flag = 0;
    // if (di->type == 1
    // || di->type == 2
    // || di->type == 3) {
    // flag = 1;
    // }

    if (di->type > 0 && di->type < 4) {
      // if (flag == 1) {
      int ok1 = 0;
      while (ok1 < NDIRECT) {
        if (di->addrs[ok1] != 0) {
          inc++;
        }
        if (di->addrs[ok1] == 0) {
          break;
        }
        ok1++;
      }

      if (((di->addrs[NDIRECT] * BSIZE) + img) != 0) {
        int ok = 0;
        while (ok < NINDIRECT) {
          ind = (uint *) ((ok * sizeof(uint))
           + (di->addrs[NDIRECT] * BSIZE) + img);
          if (*ind != 0) {
            inc++;
          }
          ok++;
       }
      }
      assert(!((di->size > inc * BSIZE || di->size < ((inc - 1) * BSIZE))
       && (inc != 0)), "ERROR: bad size in inode.\n");
    }
    di++;
    cnt++;
  }
  // INODE CHECK 2 END---------------------------------------------

  // FREE
  munmap(img, size);
  close(fd);
  exit(0);
}
