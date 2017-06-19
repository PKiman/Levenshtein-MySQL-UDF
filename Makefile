ARCH := $(shell getconf LONG_BIT)

CPP_FLAGS_32 := -m32
CPP_FLAGS_64 := -m64

CPP_FLAGS := $(CPP_FLAGS_$(ARCH)) -Wall -fPIC -pipe -O3

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
    MYSQL_CFLAGS := -I/opt/local/include/mysql56/mysql/
	CPP_FLAGS += -v -bundle $(MYSQL_CFLAGS)
endif
ifeq ($(detected_OS),Windows)
    UNITTEST_SRC := test\similarities_test.c
    UNITTEST_DST := test\similarities_test.exe
    MYSQL_CFLAGS := -IC:\mysql\include\
	CPP_FLAGS += -v -shared $(MYSQL_CFLAGS)
	TARGET := similarities.dll
    UNITTEST_DST := similarities_test.exe
endif
ifeq ($(detected_OS),Linux)
    MYSQL_CFLAGS := -I/usr/include/mysql/
	CPP_FLAGS += -v -shared $(MYSQL_CFLAGS)
endif

all: similarities similarities_test

similarities: $(SOURCES)
	$(CC) $(CPP_FLAGS) -o $(TARGET) $(SOURCES)

similarities_test: $(UNITTEST_SRC)
	$(CC) $(MYSQL_CFLAGS) -o $(UNITTEST_DST) $(UNITTEST_SRC)

clean:
	-rm -v $(TARGET) $(UNITTEST_DST)

test: run

run:
	./test/similarities_test
