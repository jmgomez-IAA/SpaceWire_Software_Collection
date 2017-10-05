# The output target
TARGET = test_loopback 

SRC = test_loopback.c    \
	utility.c
CC = gcc

ifdef DEBUG
	CFLAGS += -g -O0
else
	CFLAGS += -O3
endif

CFLAGS += -Wall 
LIBS_PATH = -I/usr/local/STAR-Dundee/STAR-System/inc/star/
LIBS = -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api

OBJS = $(addsuffix .o, $(basename $(SRC)))

all:  $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(LIBS_PATH) -c $< -o $@

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(LIBS_PATH) $^ $(LIBS) $(LINK_FLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
