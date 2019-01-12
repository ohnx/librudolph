OUTPUT:=librudolph.a
CFLAGS+=-Wall -Werror -Iinclude/ -ansi -pedantic -DRUDOLF_USE_STDLIB
SRCS:=$(wildcard src/*.c)
OBJS:=$(patsubst src/%.c, objs/%.o, $(SRCS))

.PHONY: default
default: $(OUTPUT)

objs/%.o: src/%.c
	@mkdir -p objs/
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTPUT): $(OBJS)
	ar rcs $(OUTPUT) $^

.PHONY: clean
clean:
	rm -rf objs/ $(OUTPUT)

.PHONY: debug
debug: CFLAGS += -D__DEBUG -g -O0
debug: default


.PHONY: test
test: debug
	$(CC) -o $@ tests/main.c $(OUTPUT) $(CFLAGS)
