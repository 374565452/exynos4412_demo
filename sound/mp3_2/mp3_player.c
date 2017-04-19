
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


#include "mp3_pcm.h"
#include "mp3_mad.h"


/*
* This is perhaps the simplest example use of the MAD high-level API.
* Standard input is mapped into memory via mmap(), then the high-level API
* is invoked with three callbacks: input, output, and error. The output
* callback converts MAD's high-resolution PCM samples to 16 bits, then
* writes them to standard output in little-endian, stereo-interleaved
* format.
*/
//static int decode(mp3_file *mp3fp);
int main(int argc, char *argv[])
{
    printf("------------------------------------------------------------");
    long flen, fsta, fend;
    int dlen;
    mp3_file *mp3fp;
    if (argc < 2)
        return 1;
    mp3fp = (mp3_file *)malloc(sizeof(mp3_file));
    if((mp3fp->fp = fopen(argv[1], "r")) == NULL)
    {
        printf("can't open source file.\n");
        return 2;
    }
    fsta = ftell(mp3fp->fp);
    fseek(mp3fp->fp, 0, SEEK_END);
    fend = ftell(mp3fp->fp);
    flen = fend - fsta;
    fseek(mp3fp->fp, 0, SEEK_SET);
    fread(mp3fp->fbuf, 1, BUFSIZE, mp3fp->fp);
    mp3fp->fbsize = BUFSIZE;
    mp3fp->fpos = BUFSIZE;
    mp3fp->flen = flen;
    //set_dsp();
    
     if(set_pcm()!=0)                 //设置pcm 参数
    {
        printf("set_pcm fialed:\n");
        return 1;   
    }
    init_mixer();
    int cur_vol=read_vol();
    printf("the default vol is  %d \r\n",cur_vol);
    int vol = 90;
    if((argc == 3))
    {
        vol =atoi(argv[2]);
        printf("the vol is %d \r\n",vol);
    }
    write_vol(vol);
    cur_vol=read_vol();
    printf("the after config vol is %d \r\n",cur_vol);

    decode(mp3fp);
    player_run();
    close_decode();

    //close(soundfd);
    fclose(mp3fp->fp);
    free(mp3fp);
    return 0;
}
