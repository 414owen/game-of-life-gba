ARCH := -mcpu=arm7tdmi
WARNS := -Wall -pedantic
CFLAGS := -O2 -Wall -fdiagnostics-show-option -march=armv4t -Wno-switch -Wno-multichar -ffast-math $(ARCH) -mtune=arm7tdmi -marm -mlong-calls -flto ${CFLAGS}
LDFLAGS = -nostartfiles -Tlnkscript
DEBUG ?= false

objs := lib/crt0.o lib/font.o lib/input.o lib/halt.o gen/boards_packed.o gen/boards_rle.o

# build/gen/boards_rle.o build/gen/boards_packed.o

# for some reason main.c has to be the last argument?
a.out: $(objs) main.c include/*.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(objs) main.c

test: a.out
	mgba-qt -3 a.out

# generated by c code
gen/%.c: build/gen_boards all/*
	mkdir -p gen
	./build/gen_boards

# generated by re2c
gen/scanners/%.c: misc/%.r2c
	mkdir -p gen/scanners
	re2c $< -o $@

build/scanners/%.o: gen/scanners/%.c
	mkdir -p build/scanners
	clang $(WARNS) -c -O1 -g -o $@ $^ -Wno-discarded-qualifiers

build/gen/%.o: gen/%.c
	mkdir -p build/gen
	$(CC) $(CFLAGS) -s -c -o $@ $^

build/gen_boards: build/scanners/rle_header.o misc/gen_boards.c
	# -fsanitize=address -fno-omit-frame-pointer
	clang $(WARNS) -g -O1 -o $@ $^

lib/%.o: src/%.c
	mkdir -p lib
	$(CC) $(CFLAGS) -c -o $@ $<

lib/%.o: src/%.s
	mkdir -p lib
	$(AS) -o $@ $< $(ASFLAGS)

clean:
	rm -rf *.gba lib *.out *.elf *.sav
	rm -rf build gen

a.gba: a.out
	$(OBJCOPY) -O binary a.out a.gba

life.gba: a.gba
	./ht.pl -n "Game of Life" -clo life.gba a.gba
