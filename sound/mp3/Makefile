#CC = arm-none-linux-gnueabi-gcc
#OBJECTS = alsa_pcm.o cycle_buffer.o mp3_audio.o mp3_player.o player.o

#player : $(OBJECTS)
#	$(CC) -o player $(OBJECTS)  -lpthread -lmad -lasound -L /usr/local/arm-alsa/lib/ -I/usr/local/arm-alsa/include/

#alsa_pcm.o : alsa_pcm.h 

#cycle_buffer.o : cycle_buffer.h

#mp3_audio.o : cycle_buffer.h mp3_audio.h

#mp3_player.o : cycle_buffer.h mp3_audio.h alsa_pcm.h mp3_player.h

#player.o : cycle_buffer.h mp3_audio.h alsa_pcm.h mp3_player.h
CC = arm-none-linux-gnueabi-gcc
CFLAGS = -I/usr/local/arm-alsa/include/

LIBS = -L/usr/local/arm-alsa/lib/ -lmad -lasound -lpthread
SRCS = player.c alsa_pcm.c cycle_buffer.c mp3_audio.c mp3_player.c 
OBJECTS = $(SRCS:.c=.o)
EXECUTABLE = player

all:
	$(CC)  -o $(EXECUTABLE) $(SRCS) $(LIBS) $(CFLAGS)

.PHONY : clean

clean :
	rm -rf $(OBJECTS)