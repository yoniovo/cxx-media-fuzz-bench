CXX = clang++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iinclude
ASAN_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer -g
FUZZER_FLAGS = -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -g

SRC = src/riff_parser.cpp src/media_stream.cpp

all: lib/libtinymedia.a bin/unit_tests

lib/libtinymedia.a: $(SRC)
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) -c src/riff_parser.cpp -o src/riff_parser.o
	$(CXX) $(CXXFLAGS) -c src/media_stream.cpp -o src/media_stream.o
	ar rcs $@ src/riff_parser.o src/media_stream.o

bin/unit_tests: tests/unit_tests.cpp lib/libtinymedia.a
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(ASAN_FLAGS) tests/unit_tests.cpp -Llib -ltinymedia -o $@

fuzzers:
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(FUZZER_FLAGS) $(SRC) fuzzer/fuzzer_riff.cpp -o bin/fuzzer_riff
	$(CXX) $(CXXFLAGS) $(FUZZER_FLAGS) $(SRC) fuzzer/fuzzer_stream.cpp -o bin/fuzzer_stream

test: bin/unit_tests
	./bin/unit_tests

clean:
	rm -rf bin lib src/*.o crash-* leak-* timeout-* *.dSYM
