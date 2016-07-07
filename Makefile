CXXFLAGS = -fPIC -Wall -Wconversion -Werror -Wextra \
	   -Wformat=2 -Wno-unused-function -Wpointer-arith \
	   -Wshadow -Wsign-conversion -Wstrict-overflow=5

LIBRARIES = -L/usr/lib/x86_64-linux-gnu -lboost_system

OBJECT_FILES = mcnoodle.o

all:	$(OBJECT_FILES)
	$(CXX) $(CXXFLAGS) $(LIBRARIES) mcnoodle.cc test.cc -o test

clean:
	rm -f *.o
	rm -f test

purge:
	rm -f *~*
