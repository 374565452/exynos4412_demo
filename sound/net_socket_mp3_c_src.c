#include "mad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#define BUFSIZE 8192


/**
 * 网络上下载的，用于制作mp3程序所用-----主要是用来参考写程序
 */

/*
* This is a private message structure. A generic pointer to this structure
* is passed to each of the callback functions. Put here any data you need
* to access from within the callbacks.
*/  
struct buffer {
    FILE *fp; /*file pointer*/
    unsigned int flen; /*file length*/
    unsigned int fpos; /*current position*/
    unsigned char fbuf[BUFSIZE]; /*buffer*/
    unsigned int fbsize; /*indeed size of buffer*/
};
typedef struct buffer mp3_file;
int soundfd; /*soundcard file*/
unsigned int prerate = 0; /*the pre simple rate*/
int writedsp(int c)
{
    return write(soundfd, (char *)&c, 1);
}
void set_dsp()
{
    int rate = 44100;
//  int rate = 96000;
  int format = AFMT_S16_LE;
    int channels = 2;
    int value;
    soundfd = open("/dev/dsp", O_WRONLY);
                                                                                                                                                                                                                                                                                                                                                                                                             
    ioctl(soundfd,SNDCTL_DSP_SPEED,&rate);
    ioctl(soundfd, SNDCTL_DSP_SETFMT, &format);
    ioctl(soundfd, SNDCTL_DSP_CHANNELS, &channels);
/*
    value = 16;
    ioctl(soundfd,SNDCTL_DSP_SAMPLESIZE,&value);
    value = 0;
    ioctl(soundfd,SNDCTL_DSP_STEREO,&value);
*/
}
/*
* This is perhaps the simplest example use of the MAD high-level API.
* Standard input is mapped into memory via mmap(), then the high-level API
* is invoked with three callbacks: input, output, and error. The output
* callback converts MAD's high-resolution PCM samples to 16 bits, then
* writes them to standard output in little-endian, stereo-interleaved
* format.
*/
static int decode(mp3_file *mp3fp);
int main(int argc, char *argv[])
{
    long flen, fsta, fend;
    int dlen;
    mp3_file *mp3fp;
    if (argc != 2)
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
    set_dsp();
    decode(mp3fp);
    close(soundfd);
    fclose(mp3fp->fp);
    return 0;
}
/*
* This is the input callback. The purpose of this callback is to (re)fill
* the stream buffer which is to be decoded. In this example, an entire file
* has been mapped into memory, so we just call mad_stream_buffer() with the
* address and length of the mapping. When this callback is called a second
* time, we are finished decoding.
*/
static
enum mad_flow input(void *data,
struct mad_stream *stream)
{
    mp3_file *mp3fp;
    int ret_code;
    int unproc_data_size; /*the unprocessed data's size*/
    int copy_size;
    mp3fp = (mp3_file *)data;
    if(mp3fp->fpos <= mp3fp->flen)
    {
        unproc_data_size = stream->bufend - stream->next_frame;
        memcpy(mp3fp->fbuf, mp3fp->fbuf+mp3fp->fbsize-unproc_data_size, unproc_data_size);
        copy_size = BUFSIZE - unproc_data_size;
        if(mp3fp->fpos + copy_size > mp3fp->flen)
        {
            copy_size = mp3fp->flen - mp3fp->fpos;
        }
        fread(mp3fp->fbuf+unproc_data_size, 1, copy_size, mp3fp->fp);
        mp3fp->fbsize = unproc_data_size + copy_size;
        mp3fp->fpos += copy_size;
        /*Hand off the buffer to the mp3 input stream*/
        mad_stream_buffer(stream, mp3fp->fbuf, mp3fp->fbsize);
        ret_code = MAD_FLOW_CONTINUE;
    }
    else
    {
        ret_code = MAD_FLOW_STOP;
    }
    return ret_code;
}
/*
* The following utility routine performs simple rounding, clipping, and
* scaling of MAD's high-resolution samples down to 16 bits. It does not
* perform any dithering or noise shaping, which would be recommended to
* obtain any exceptional audio quality. It is therefore not recommended to
* use this routine if high-quality output is desired.
*/
static inline
signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L <= MAD_F_FRACBITS - 16);
    if(sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if(sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}
/*
* This is the output callback function. It is called after each frame of
* MPEG audio data has been completely decoded. The purpose of this callback
* is to output (or play) the decoded PCM audio.
*/
static
enum mad_flow output(void *data,
struct mad_header const *header,
struct mad_pcm *pcm)
{
    unsigned int nchannels, nsamples;
    unsigned int rate;
    mad_fixed_t const *left_ch, *right_ch;
    /* pcm->samplerate contains the sampling frequency */
    rate = pcm->samplerate;
    nchannels = pcm->channels;
    nsamples = pcm->length;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];
    /* update the sample rate of dsp*/
    if(rate != prerate)
    {
        ioctl(soundfd, SNDCTL_DSP_SPEED, &rate);
        prerate = rate;
    }
    while (nsamples--)
    {
        signed int sample;
        /* output sample(s) in 16-bit signed little-endian PCM */
        sample = scale(*left_ch++);
        writedsp((sample >> 0) & 0xff);
        writedsp((sample >> 8) & 0xff);
        if (nchannels == 2)
        {
            sample = scale(*right_ch++);
            writedsp((sample >> 0) & 0xff);
            writedsp((sample >> 8) & 0xff);
        }
    }
    return MAD_FLOW_CONTINUE;
}
/*
* This is the error callback function. It is called whenever a decoding
* error occurs. The error is indicated by stream->error; the list of
* possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
* header file.
*/
static enum mad_flow error(void *data,
struct mad_stream *stream,
struct mad_frame *frame)
{
    mp3_file *mp3fp = data;
    fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
    stream->error, mad_stream_errorstr(stream),
    stream->this_frame - mp3fp->fbuf);
    /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */
    return MAD_FLOW_CONTINUE;
}
/*
* This is the function called by main() above to perform all the decoding.
* It instantiates a decoder object and configures it with the input,
* output, and error callback functions above. A single call to
* mad_decoder_run() continues until a callback function returns
* MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
* signal an error).
*/
static int decode(mp3_file *mp3fp)
{
    struct mad_decoder decoder;
    int result;
    /* configure input, output, and error functions */
    mad_decoder_init(&decoder, mp3fp,
    input, 0 /* header */, 0 /* filter */, output,
    error, 0 /* message */);
    /* start decoding */
    result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
    /* release the decoder */
    mad_decoder_finish(&decoder);
    return result;
}