SER = HudpServer.cpp
CLI = HudpClient.cpp

CC = g++

INCLUDES = -I../../base      \
           -I../../include   \

#debug
CCFLAGS = -lpthread -fPIC -m64 -g -pg -std=c++11 -lstdc++ -pipe 

#CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe -march=corei7 

TARGET = ../../libhudp.a
SERBIN = server
CLIBIN = client

all:$(SERBIN) $(CLIBIN)

$(SERBIN):$(SER)
	$(CC) $(SER) -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

$(CLIBIN):$(CLI)
	$(CC) $(CLI) -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

clean:
	rm -rf $(SERBIN) $(CLIBIN)