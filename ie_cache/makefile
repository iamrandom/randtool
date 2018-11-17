


CC:=gcc -m32 -static-libgcc

ie_cache.exe:ie_cache.o
	$(CC) -o $@ $^ -lwininet

parse_cache_txt.exe:parse_cache_txt.o
	$(CC) -o $@ $^

check_swf.exe:check_swf.c
	$(CC) -o $@ $^

make_map.o: make_map.c
	$(CC) -m64 -g -o $@ -c $^ -I"include"

make_map.exe: make_map.o
	$(CC) -m64 -g -o  $@ $^ -L"lib"  -ljpeg -lpng -lz -lglui -lglut32  -lglu32 -mwindows


.PHONY:clean
clean:
	rm -f *.exe *.o

