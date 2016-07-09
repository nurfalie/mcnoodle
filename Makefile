CXXFLAGS = -fPIC -Wall -Wconversion -Werror -Wextra \
	   -Wformat=2 -Wno-unused-function -Wpointer-arith \
	   -Wshadow -Wstrict-overflow=5 \
	   -mtune=native

LIBRARIES = -lboost_random -lboost_serialization -lboost_system

OBJECT_FILES = mcnoodle.o

all:	$(OBJECT_FILES)
	$(CXX) $(CXXFLAGS) mcnoodle.cc test.cc -o test $(LIBRARIES)

clean:
	rm -f *.o
	rm -f test

purge:
	rm -f *~*
