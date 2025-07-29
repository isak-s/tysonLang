CC = cc
CFLAGS = -std=c99 -Wall -Ilib/mpc
SRC = src/tysonlang.c lib/mpc/mpc.c
LIBS = -ledit -lm
OUT = tysonlang

all:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(OUT)

clean:
	rm -f $(OUT)

tyson:
	@./$(OUT) lib-tyson/std.tyson $(filter-out $@,$(MAKECMDGOALS))

%:
	@: