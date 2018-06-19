cxx = g++
cc = gcc

target = project2
generator = input_generator
log = *.log

cxxsrcs = main.cpp zynq.cpp fpga.cpp
cxxobjs = $(cxxsrcs:.cpp=.o)
cxxhdrs = zynq.h fpga.h

#uncomment -DDEBUG to print values
cxxflags = -std=c++14 -O2 -Wall -Werror -Wno-pointer-arith -DDEBUG

csrcs = generator.c
cobjs = $(csrcs:.c=.o)
chdrs = 

cflags = -std=c99 -O2 -Wall -Werror

includes = -I.
libs=
ldflags = 

all: $(target) $(generator)

$(target):	$(cxxobjs)
		$(cxx) $(cxxobjs) -o $(target) $(libs) $(ldflags)

.cpp.o:
	$(cxx) $(cxxflags) $(includes) -c $< -o $@

$(generator):	$(cobjs)
		$(cc) $(cobjs) -o $(generator) $(libs) $(ldflags)

.c.o:
	$(cc) $(cflags) $(includes) -c $< -o $@

clean:
	rm -f $(cxxobjs) $(cobjs) $(target) $(generator) $(log)

tar:
	tar -czvf $(target).tar.gz $(cxxsrcs) $(cxxhdrs) $(csrcs) $(chdrs) input.txt Makefile
