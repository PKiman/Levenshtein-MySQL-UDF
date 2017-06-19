ARCH := $(shell getconf LONG_BIT)

CFLAGS_32 := -m32
CFLAGS_64 := -m64

CFLAGS := $(CFLAGS_$(ARCH)) -Wall -fPIC -pipe -O3

CC=gcc
TARGET := similarities.so
SOURCES := similarities.c
UNITTEST_SRC := test/similarities_test.c
UNITTEST_DST := test/similarities_test

ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname -s)
endif
ifeq ($(detected_OS),Darwin)  # Mac OS X
	MYSQL_CFLAGS ?= -I/opt/local/include/mysql56/mysql/
	CFLAGS += -v -bundle $(MYSQL_CFLAGS)
endif
ifeq ($(detected_OS),Windows)
	UNITTEST_SRC := test\similarities_test.c
	UNITTEST_DST := test\similarities_test.exe
 	MYSQL_CFLAGS ?= -IC:\mysql\include\
	CFLAGS += -v -shared $(MYSQL_CFLAGS)
	TARGET := similarities.dll
	UNITTEST_DST := similarities_test.exe
endif
ifeq ($(detected_OS),Linux)
	MYSQL_CFLAGS ?= -I/usr/include/mysql/
	CFLAGS += -v -shared $(MYSQL_CFLAGS)
endif

all: similarities similarities_test

similarities: $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

similarities_test: $(UNITTEST_SRC)
	$(CC) $(MYSQL_CFLAGS) -o $(UNITTEST_DST) $(UNITTEST_SRC)

clean:
	-rm -v $(TARGET) $(UNITTEST_DST)

test: run

run:
	./test/similarities_test
