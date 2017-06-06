OBJ_DIR = obj
SRC_DIR =  src
OBJ = Connection.o KinectWrapper.o
OBJ_SERVER = $(patsubst %.o, $(OBJ_DIR)/%.o, MainServer.o $(OBJ))
OBJ_CLIENT = $(patsubst %.o, $(OBJ_DIR)/%.o, MainClient.o $(OBJ))
OBJ_TEST_KINECT = $(patsubst %.o, $(OBJ_DIR)/%.o, TestKinect.o $(OBJ))
OBJ_TEST_SERVER = $(patsubst %.o, $(OBJ_DIR)/%.o, TestServer.o $(OBJ))

CC = g++
CPP_VERSION = -std=c++11
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG) -L/usr/local/lib/
LIBS = -lfreenect -lfreenect_sync
CV_LIBS = `pkg-config --libs --cflags opencv`

server : $(OBJ_SERVER)
	$(CC) $(CPP_VERSION) -o server $(OBJ_SERVER) $(LFLAGS) $(LIBS) $(CV_LIBS)

client : $(OBJ_CLIENT)
	$(CC) $(CPP_VERSION) -o client $(OBJ_CLIENT) $(LFLAGS) $(LIBS)

test_kinect : $(OBJ_TEST_KINECT)
	$(CC) $(CPP_VERSION) -o test_kinect $(OBJ_TEST_KINECT) $(LFLAGS) $(LIBS)

test_server : $(OBJ_TEST_SERVER)
	$(CC) $(CPP_VERSION) -o test_server $(OBJ_TEST_SERVER) $(LFLAGS) $(LIBS)

test_pic : obj/_Test.o
	$(CC) $(CPP_VERSION) -o test_pic $(OBJ_DIR)/_Test.o $(LFLAGS) $(LIBS) $(CV_LIBS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	\rm -f $(OBJ_DIR)/*.o server client test_kinect test_server test_pic