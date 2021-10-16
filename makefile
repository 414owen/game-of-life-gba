ARCH := -mcpu=arm7tdmi
CFLAGS := -g -O2 -Wall -march=armv4t -Wno-switch -Wno-multichar -ffast-math $(ARCH) -mtune=arm7tdmi -marm -faggressive-loop-optimizations -mlong-calls -Iinclude -flto ${CFLAGS} 
LDFLAGS = -nostartfiles -Tlnkscript
DEBUG ?= false

GBA_LIBS = lib/crt0.o lib/font.o lib/boards_compressed.o lib/input.o lib/halt.o

a.out: main.c $(GBA_LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(GBA_LIBS) main.c

test: a.out
	mgba -3 a.out

test_rle:
	re2c misc/rle.r2c -o build/rle.c
	gcc -g -O0 -o build/rle build/rle.c -Iinclude
	./build/rle all/worm.rle

lib/%.o: src/%.c
	mkdir -p lib
	$(CC) -c -o $@ $< $(CFLAGS)

lib/%.o: src/%.s
	mkdir -p lib
	$(AS) -o $@ $< $(ASFLAGS)

src/boards_compressed.c: misc/compress_boards.c
	mkdir -p build
	gcc -Wall -O1 -Iinclude misc/compress_boards.c src/boards.c -o build/generate_compressed_builds
	./build/generate_compressed_builds

clean:
	rm -rf *.gba lib *.out *.elf *.sav
	rm -rf build
	rm -f src/boards_compressed.c

a.gba: a.out
	$(OBJCOPY) -O binary a.out a.gba

game.gba: a.gba
	./ht.pl -n "Game of Life" -clo game.gba a.gba
