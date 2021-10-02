#include <stdint.h>

uint16_t *fb = (void*)0x6000000;
const int xsz = 240;
const int ysz = 160;

#define FRAME_SEL_BIT		0x10
#define BG2_ENABLE			0x400

int main(void) {
	int i;
	static volatile uint16_t * const reg_disp_ctl = (void*)0x4000000;

	*reg_disp_ctl = 3 | BG2_ENABLE;
	*reg_disp_ctl &= ~FRAME_SEL_BIT;

	for(i=0; i<xsz * ysz; i++) {
		fb[i] = 31;
	}

	for(;;);
	return 0;
}
