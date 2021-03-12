SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)
TEST_SOURCES := $(wildcard test/*.cpp)

bson_reader: $(SOURCES) $(HEADER) bson_reader.c
	gcc -g -O0 -Isrc $^ -o $@

.PHONY: clean
clean:
	rm -fr bson_reader

.PHONY: format
format:
	clang-format --style=file -i $(HEADERS) $(SOURCES) $(TEST_SOURCES)
