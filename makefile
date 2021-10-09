ARCH := -mcpu=arm7tdmi
CFLAGS := -Wall -O2 $(ARCH) -mtune=arm7tdmi
ASFLAGS := $(ARCH)
LDFLAGS = -nostartfiles -Tlnkscript

a.out: main.c lib/crt0.o lib/font.o lib/boards.o
	$(CC) $(CFLAGS) $(LDFLAGS) lib/*.o main.c

test: a.out
	mgba -3 a.out

lib/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

lib/%.o : src/%.s
	$(AS) -o $@ $< $(ASFLAGS)

clean:
	rm -f *.gba lib/*.o *.out *.elf *.sav

a.gba : a.out
	$(OBJCOPY) -O binary a.out a.gba

game.gba: a.gba
	./ht.pl -n "Game of Life" -clo game.gba a.gba
