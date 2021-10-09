ARCH := -mcpu=arm7tdmi
CFLAGS := -Wall -O2 $(ARCH) -mtune=arm7tdmi
ASFLAGS := $(ARCH)
LDFLAGS = -nostartfiles -Tlnkscript

a.out: main.c crt0.o input.o
	$(CC) $(CFLAGS) $(LDFLAGS) crt0.o input.o main.c

input.o: input.c
	$(CC) -c -o $@ $< $(CFLAGS)

crt0.o : crt0.s
	$(AS) -o $@ $< $(ASFLAGS)

clean:
	rm -f *.gba *.o *.out *.elf *.sav

a.gba : a.out
	$(PREFIX)-objcopy -O binary a.out a.gba

game.gba: a.gba
	./ht.pl -n "Game of Life" -clo game.gba a.gba
