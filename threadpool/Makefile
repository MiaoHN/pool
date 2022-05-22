CC     = g++
CFLAGS = -Wall
LIBS   = -pthread
TARGET = threadpool.out

.PHONY: clean build all

all: build

build:
	@$(CC) main.cpp $(CFLAGS) $(LIBS) -o $(TARGET)
	@echo 'build done.'

run: build
	@echo 'start running...'
	@./$(TARGET)

clean:
	@rm $(TARGET)
	@echo 'clean done.'
