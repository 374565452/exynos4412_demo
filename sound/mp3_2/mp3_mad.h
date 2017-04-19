#ifndef __MP3_MAD_H	
#define __MP3_MAD_H

# include <stdio.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/mman.h>

# include "mad.h"


#define BUFSIZE 8192

struct buffer {
    FILE *fp; /*file pointer*/
    unsigned int flen; /*file length*/
    unsigned int fpos; /*current position*/
    unsigned char fbuf[BUFSIZE]; /*buffer*/
    unsigned int fbsize; /*indeed size of buffer*/
};
typedef struct buffer mp3_file;


int decode(mp3_file *mp3fp);

void close_decode(); 

void player_run();

#endif