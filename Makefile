CPP:=g++

SRCS:=\
	main.cpp \
	formats/format.cpp \
	formats/format_twobit.cpp
OBJS:=$(SRCS:.cpp=.o)

.PHONY: default
default: ggt

ggt: $(OBJS)
	$(CPP) -o $@ $+

.cpp.o:
	$(CPP) -c -o $@ $<

clean:
	rm -f *~ */*~ */*.o ggt
