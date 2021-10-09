ARCH := -mcpu=arm7tdmi
CFLAGS := -Wall -O2 $(ARCH) -mtune=arm7tdmi
ASFLAGS := $(ARCH)
LDFLAGS = -nostartfiles -Tlnkscript

GBA_LIBS = lib/crt0.o lib/font.o lib/boards_compressed.o

a.out: main.c $(GBA_LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(GBA_LIBS) main.c

test: a.out
	mgba -3 a.out

lib/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

lib/%.o: src/%.s
	$(AS) -o $@ $< $(ASFLAGS)

src/boards_compressed.c: misc/compress_boards.c
	mkdir -p build
	gcc -Wall -g -O1 misc/compress_boards.c src/boards.c -o build/generate_compressed_builds
	./build/generate_compressed_builds

clean:
	rm -f *.gba lib/*.o *.out *.elf *.sav
	rm -rf build

a.gba: a.out
	$(OBJCOPY) -O binary a.out a.gba

game.gba: a.gba
	./ht.pl -n "Game of Life" -clo game.gba a.gba
