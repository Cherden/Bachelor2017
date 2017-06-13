CXX 			= 	g++
PROTO_CC 		= 	protoc
CXX_VERSION 	= 	-std=c++11
CFLAGS 			= 	-Wall -c -g
LFLAGS 			= 	-Wall -g -L/usr/local/lib/
LIBS 			= 	-lfreenect -lfreenect_sync -lprotobuf `pkg-config --libs --cflags opencv`


OBJ_DIR 		=	obj
SRC_DIR 		=	src
PROTO_DIR 		= 	gen

PROTO_OBJS		= 	$(patsubst $(PROTO_DIR)/%.proto, $(OBJ_DIR)/%.pb.o, $(wildcard $(PROTO_DIR)/*.proto))

OBJ 			=	Connection.o KinectWrapper.o
OBJ_SERVER 		=	$(PROTO_OBJS) $(patsubst %.o, $(OBJ_DIR)/%.o, MainServer.o $(OBJ))
OBJ_CLIENT		= 	$(PROTO_OBJS) $(patsubst %.o, $(OBJ_DIR)/%.o, MainClient.o $(OBJ))
OBJ_TEST_KINECT	= 	$(patsubst %.o, $(OBJ_DIR)/%.o, TestKinect.o $(OBJ))
OBJ_TEST_SERVER = 	$(patsubst %.o, $(OBJ_DIR)/%.o, TestServer.o $(OBJ))
OBJ_TEST_PIC	= 	$(PROTO_OBJS) $(patsubst %.o, $(OBJ_DIR)/%.o, TestPic.o $(OBJ))



server : $(OBJ_SERVER)
	$(CXX) $(CXX_VERSION) -o server $(OBJ_SERVER) $(LFLAGS) $(LIBS)

client : $(OBJ_CLIENT)
	$(CXX) $(CXX_VERSION) -o client $(OBJ_CLIENT) $(LFLAGS) $(LIBS)

test_kinect : $(OBJ_TEST_KINECT)
	$(CXX) $(CXX_VERSION) -o test_kinect $(OBJ_TEST_KINECT) $(LFLAGS) $(LIBS)

test_server : $(OBJ_TEST_SERVER)
	$(CXX) $(CXX_VERSION) -o test_server $(OBJ_TEST_SERVER) $(LFLAGS) $(LIBS)

test_pic : $(OBJ_TEST_PIC)
	$(CXX) $(CXX_VERSION) -o test_pic $(OBJ_TEST_PIC) $(LFLAGS) $(LIBS)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) $< -o $@

$(PROTO_DIR)/%.pb.h $(PROTO_DIR)/%.pb.cc : $(PROTO_DIR)/%.proto
	$(PROTO_CC) -I./$(PROTO_DIR) --cpp_out=$(PROTO_DIR) $<

$(OBJ_DIR)/%.pb.o : $(PROTO_DIR)/%.pb.cc
	$(CXX) $(CFLAGS) $< -o $@


clean:
	\rm -f $(OBJ_DIR)/*.o $(PROTO_DIR)/*.pb.* server client test_kinect test_server test_pic
