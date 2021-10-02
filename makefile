PREFIX = arm-none-eabi

game.gba: a.gba
	./ht.pl -n "Game of Life" -clo game.gba a.gba

a.gba: a.out
	$(PREFIX)-objcopy -O binary a.out a.gba

a.out: main.c
	$(PREFIX)-gcc crt0.s main.c -mcpu=arm7tdmi -nostartfiles -Tlnkscript

clean:
	rm *.out *.gba
