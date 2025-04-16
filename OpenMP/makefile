# Makefile for a basic OpenMP project on macOS
# gcc-14 -fopenmp -Wall -o work1_sol1 work1_sol1.c

SRC = sharpeRatio.c         
EXE = $(basename $(SRC))
CC = gcc
CCFLAGS = -Wall -fopenmp -lm

OS = $(shell uname)

ifeq ($(OS), Linux)
	CCFLAGS += -lX11 -lGL -lglfw
endif

ifeq ($(OS), Darwin)
	CCFLAGS += -lglfw -framework OpenGL -L /usr/local/Cellar/glfw/3.4/lib -I /usr/local/Cellar/glfw/3.4/include
endif

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CCFLAGS) -o $(EXE) $(SRC)

clean:
	rm -f $(EXE)
