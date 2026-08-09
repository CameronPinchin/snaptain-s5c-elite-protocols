CC=gcc
LIBS=-lssl -lcrypto

TARGET_LOGIN_REQUEST = login
TARGET_PACKET_CONSTRUCTOR = packet_const

C_SRC_LOGIN=login_request.c
C_SRC_PACKET=packet_constructor_test.c

_dummy := $(shell mkdir -p bin)

all: $(TARGET_LOGIN_REQUEST) $(TARGET_PACKET_CONSTRUCTOR)

$(TARGET_PACKET_CONSTRUCTOR):
	$(CC) -g $(C_SRC_PACKET) -o bin/$(TARGET_PACKET_CONSTRUCTOR) $(LIBS)

$(TARGET_LOGIN_REQUEST):
	$(CC) -g $(C_SRC_LOGIN) -o bin/$(TARGET_LOGIN_REQUEST) $(LIBS)

clean:
	rm -f bin/*
