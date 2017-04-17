#include "mp3_player.h"

struct mad_decoder decoder;
int result;
unsigned char mp3_decoder[DECODER_LEN];

static enum mad_flow input(void *data,struct mad_stream *stream);
static inline signed int scale(mad_fixed_t sample);
static enum mad_flow output(void *data, struct mad_header const *header,struct mad_pcm *pcm);
static enum mad_flow error(void *data,struct mad_stream *stream,struct mad_frame *frame);

int decode(c_buffer * mp3)
{
	mad_decoder_init(&decoder, mp3,
		input, 0 /* header */, 0 /* filter */, output,
	    error, 0 /* message */);
}

void close_decode()
{
	mad_decoder_finish(&decoder);
}

void player_run()
{
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	printf("the result is %d -------\r\n",result );
}


static enum mad_flow input(void *data,struct mad_stream *stream)
{
	int unproc_data_size; /*the unprocessed data's size*/
    int copy_size;
    int ret_code;
    c_buffer * p =data;
    int has=get_data_size(p);
    if( !has)
    {
    	memset(mp3_decoder,0,DECODER_LEN);
    	copy_size=read_audio_data(mp3_decoder,DECODER_LEN);
    	mad_stream_buffer(stream, mp3_decoder,copy_size);
		ret_code = MAD_FLOW_CONTINUE;
    }else
    {
		ret_code = MAD_FLOW_STOP;
    }
    return ret_code;
}

static inline signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L <= MAD_F_FRACBITS - 16);
    if(sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if(sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow output(void *data, struct mad_header const *header,struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples,n;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  n=nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];
  
  unsigned char Output[6912], *OutputPtr;  
  int fmt, wrote, speed, exact_rate, err, dir; 
  OutputPtr = Output;  
   while (nsamples--) 
   {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */
    
    sample = scale(*left_ch++);
   
    *(OutputPtr++) = sample >> 0;  
    *(OutputPtr++) = sample >> 8;  
    if (nchannels == 2)  
        {  
            sample = scale (*right_ch++);  
            *(OutputPtr++) = sample >> 0;  
            *(OutputPtr++) = sample >> 8;  
        }  
  }
 
 	OutputPtr = Output;  

 	//snd_pcm_writei (handle, OutputPtr, n);  
 	write_pcm(OutputPtr,n);
 	OutputPtr = Output;     

	return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data,struct mad_stream *stream,struct mad_frame *frame)
{
  //struct buffer *buffer = data;
  //printf("this is mad_flow error\n");
  //fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
    //  stream->error, mad_stream_errorstr(stream),
     // stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}