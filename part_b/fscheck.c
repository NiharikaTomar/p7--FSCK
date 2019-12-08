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


int main(int argc, char *argv[]){
	if(argc < 2 || argc > 2){
		cprintf("illegal number of arguments");
	}
	exit(1);
}