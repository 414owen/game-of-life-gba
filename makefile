CFLAGS = -mcpu=arm7tdmi
LDFLAGS = -nostartfiles -Tlnkscript

SOURCES=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SOURCES)) crt0.o

# link step: .o -> .elf
game.elf : $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

crt0.o : crt0.s
	$(AS) -o $@ $< $(CFLAGS)

# compile step .c -> .o
%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.gba *.o *.out *.elf *.sav

a.gba : game.elf
	$(PREFIX)-objcopy -O binary game.elf a.gba

game.gba: a.gba
	./ht.pl -n "Game of Life" -clo game.gba a.gba

a.out: main.c
	$(PREFIX)-gcc crt0.s main.c
