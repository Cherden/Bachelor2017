OBJS = KinectWrapper.o main.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall -L/usr/local/lib/ -lfreenect -lfreenect_sync $(DEBUG)

dist_middleware : $(OBJS)
	$(CC) $(OBJS) -o dist_middleware $(LFLAGS)

KinectWrapper.o : KinectWrapper.h KinectWrapper.cpp
	$(CC) $(CFLAGS) KinectWrapper.cpp

main.o : KinectWrapper.h KinectWrapper.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	\rm *.o dist_middleware
