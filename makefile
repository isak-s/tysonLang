CC = cc
CFLAGS = -std=c99 -Wall -Ilib/mpc
SRC = src/tysonlang.c lib/mpc/mpc.c
LIBS = -ledit -lm
OUT = tysonlang

all:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(OUT)

clean:
	rm -f $(OUT) web/tyson.js web/tyson.wasm web/tyson.html

tyson:
	@./$(OUT) lib-tyson/std.tyson $(filter-out $@,$(MAKECMDGOALS))

wasm:
	emcc $(SRC) \
		-Ilib/mpc \
		-s WASM=1 \
		-s EXPORTED_FUNCTIONS='["_tyson_init", "_eval_string"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
		-s EXIT_RUNTIME=1 \
		-o web/tyson.js

%:
	@: