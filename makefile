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

wasm:
	emcc $(SRC) \
		-Ilib/mpc \
		-s WASM=1 \
		-s EXPORTED_FUNCTIONS='["_tyson_init", "_eval_string"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
		-s EXIT_RUNTIME=1 \
		-o tyson.html

%:
	@: