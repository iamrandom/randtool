

MODE?=64
#  -fno-omit-frame-pointer
CC= gcc -m$(MODE) -std=c11 -o2  -Wall -fmessage-length=0 
CPP= g++ -m$(MODE) -std=c++11 -g -Wall -fmessage-length=0 
DFLAG?=-static-libstdc++ -static-libgcc -mwindows
CC := $(CPP) $(DFLAG)


INCLUDE =

%.o:%.cpp
	$(CC) -c $^ -o $@ $(INCLUDE)

%.exe:%.o
	$(CC)  -o $@ $^ 

.PHONY : clean
clean:
	rm -f *.exe *.o
