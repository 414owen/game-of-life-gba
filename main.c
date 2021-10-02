#define rgb(r, g, b) ((r) + (g << 5) + (b << 10))

#define WIDTH 240

#define HEIGHT 160

#define VRAM_ADDR ((unsigned short *)(0x6000000))

#define DISPCNT	(*(volatile unsigned long *)(0x4000000))

unsigned short *lcd = (unsigned short *)(VRAM_ADDR);

int main() {
	
	/* LCD MODE 3 with BG 2 ON */
	
	DISPCNT = 0x403;
	
	/* Re-draw the LCD background manually. It will be completely green. */
	
	for (unsigned x = 0; x < WIDTH; x++) {
		
		for (unsigned y = 0; y < HEIGHT; y ++) lcd[x + (y * WIDTH)] = rgb(0, 31, 0);
		
	}
	
}