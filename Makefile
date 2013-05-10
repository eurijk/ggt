CPP:=g++
CF:=-Wall

SRCS:=\
	main.cpp \
	formats/format.cpp \
	formats/format_twobit.cpp
OBJS:=$(SRCS:.cpp=.o)

.PHONY: default
default: ggt

ggt: $(OBJS)
	$(CPP) $(CF) -o $@ $+

.cpp.o:
	$(CPP) $(CF) -c -o $@ $<

clean:
	rm -f *~ */*~ */*.o ggt
