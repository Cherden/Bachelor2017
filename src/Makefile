OBJS = Logger.o UDPConnection.o KinectWrapper.o
OBJS_SERVER = MainServer.o $(OBJS)
OBJS_CLIENT = MainClient.o $(OBJS)
CC = g++
CPP_VERSION = -std=c++11
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG) -L/usr/local/lib/
LIBS = -lfreenect -lfreenect_sync

server : $(OBJS_SERVER)
	$(CC) $(CPP_VERSION) -o server $(OBJS_SERVER) $(LFLAGS) $(LIBS)

client : $(OBJS_CLIENT)
	$(CC) $(CPP_VERSION) -o client $(OBJS_CLIENT) $(LFLAGS) $(LIBS)

%.o : %.cpp
	g++ -c -g -Wall -o $@ $<
	#$(CC) $(CFLAGS) $< -o $@

clean:
	\rm *.o server client
