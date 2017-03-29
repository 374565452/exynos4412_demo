CC = arm-none-linux-gnueabi-gcc
OBJECTS = main.o net_server.o signal_pro.o

pro : $(OBJECTS)
	$(CC) -o pro $(OBJECTS) -static -lpthread

net_server.o : net_server.h commons.h

main.o : net_server.h signal_pro.h 

signal_pro.o : net_server.h signal_pro.h commons.h


.PHONY : clean

clean :
	rm -rf $(OBJECTS)