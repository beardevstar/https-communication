CC = g++
CFLAGS  = -g -Wall

ATARGET = myadmin
CTARGET = myclient
TARGET = $(ATARGET) $(CTARGET)

INCLUDES = -I./share
SRCS = ./share/app.cpp \
	./share/https_socket.cpp \
	./share/cJSON.cpp \
	./share/util.cpp

OBJS = $(SRCS:.cpp=.o)
LFALGS = -lssl -lcrypto -lpthread -lm

all: $(TARGET)
	@echo  Webapp Compiled Successfully.

$(ATARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(ATARGET) ./admin/admin.cpp $(OBJS) $(LFALGS)
$(CTARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(CTARGET) ./client/client.cpp $(OBJS) $(LFALGS)
	$(RM) ./share/*.o ./admin/*.o .client/*.o

clean:
	$(RM) ./share/*.o ./admin/*.o .client/*.o $(ATARGET) $(CTARGET)