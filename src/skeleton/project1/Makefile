cc = g++
cc2 = gcc

target = project1
generator = input_generator

csrcs = main.cpp zynq.cpp
cobjs = $(csrcs:.cpp=.o)
chdrs = zynq.h

cflags = -std=c++14 -DHLS_NO_XIL_FPO_LIB -Wall -Werror -Wno-pointer-arith -DPRINT_VALUE
includes = -I. -I./hls
libs=

all: $(target) $(generator)

$(target):	$(cobjs)
		$(cc) $(cobjs) -o $(target) $(libs) $(ldflags)

$(generator):
		$(cc2) generator.c -std=c99 -o $(generator)

.cpp.o:
	$(cc) $(cflags) $(includes) -c $< -o $@

clean:
	rm -f $(cobjs) $(target) $(generator)

tar:
	tar -czvf $(target).tar.gz $(csrcs) $(chdrs) hls generator.c input.txt Makefile
