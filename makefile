CC = cc
CFLAGS = -std=c99 -Wall
SRC = tysonlang.c mpc.c
LIBS = -ledit -lm
OUT = tysonlang

all:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(OUT)

clean:
	rm -f $(OUT)

tyson:
	@echo "Running Tyson mode..."
	./$(OUT) "Tyson-specific input"
