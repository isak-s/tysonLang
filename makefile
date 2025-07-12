CC = cc
CFLAGS = -std=c99 -Wall
SRC = parsing2.c mpc.c
LIBS = -ledit -lm
OUT = parsing2

all:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(OUT)

clean:
	rm -f $(OUT)