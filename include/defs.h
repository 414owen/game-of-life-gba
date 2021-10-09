#ifndef defs_h_INCLUDED
#define defs_h_INCLUDED

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define BIT(n) (1 << n)

#define SCREEN_WIDTH = 240
#define SCREEN_HEIGHT = 160

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char      s8;
typedef signed short     s16;
typedef signed int       s32;
typedef signed long long s64;

typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8  vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef u16 COLOR;

// typedef struct {
//   // Each element is an index into the pallete, 8bpp each
//   u32 data[16];
// } TILE8;

typedef struct {
  // Each element is an index into the pallete, 8bpp each
  union {
    u8  bytes[8][8];
    u32 data[16];
  };
} TILE8;

typedef TILE8  CHARBLOCK8[256];

// {15-12} - color palette index, for 1 pallette @ 256 colors, this is disabled
// {11}    - vertical flip
// {10}    - horizontal flip
// {9-0}   - tile index
typedef u16 SCR_ENTRY;
typedef SCR_ENTRY SCREENLINE[32];
typedef SCR_ENTRY SCREENMAT[32][32];
typedef SCR_ENTRY SCREENBLOCK[1024];

#define MEM_SYSTEM_ROM  0x00000000  // System calls
#define MEM_IO          0x04000000  // IO registers
#define MEM_PAL         0x05000000  // Pallette
#define MEM_VRAM        0x06000000  // BG and OBJ data
#define MEM_OAM         0x07000000  // Object Attribute Memory
#define MEM_PAL_BG      (MEM_PAL)   // Background palette address

#define REG_DISPCNT   *(vu32*)(MEM_IO+0x0000)  // Display control
#define REG_DISPSTAT  *(vu16*)(MEM_IO+0x0004)  // Display interupt status
#define REG_VCOUNT    *(vu16*)(MEM_IO+0x0006)  // Vertical count
#define REG_KEYINPUT  *(vu16*)(MEM_IO+0x0130)  // Key status

#define DCNT_MODE0  0x0000  // Mode 0 - bg 0-4: reg
#define DCNT_MODE1  0x0001  // Mode 1 - bg 0-1: reg; bg 2: affine
#define DCNT_MODE2  0x0002  // Mode 2 - bg 2-3: affine
#define DCNT_MODE3  0x0003  // Mode 3 - bg2: 240x160\@16 bitmap - no backbuffer
#define DCNT_MODE4  0x0004  // Mode 4 - bg2: 240x160\@8  bitmap
#define DCNT_MODE5  0x0005  // Mode 5 - bg2: 160x128\@16 bitmap

#define DCNT_BG0  0x0100  // Enable bg 0
#define DCNT_BG1  0x0200  // Enable bg 1
#define DCNT_BG2  0x0400  // Enable bg 2
#define DCNT_BG3  0x0800  // Enable bg 3

#define REG_BG0CNT  *(vu16*)(MEM_IO+0x0008)  // Bg0 control
#define REG_BG1CNT  *(vu16*)(MEM_IO+0x000A)  // Bg1 control
#define REG_BG2CNT  *(vu16*)(MEM_IO+0x000C)  // Bg2 control
#define REG_BG3CNT  *(vu16*)(MEM_IO+0x000E)  // Bg3 control

// #define BG_8BITCOL (BIT(7))
#define BG_8BITCOL 0x0080
// 0-31
#define BG_SCRNUM(n) (n << 8)
#define BG_BASENUM(n) (n << 2)

// Screenblock as matrices
// se_mat[s][y][x] = screenblock s, entry (x,y) ( SCR_ENTRY )
#define se_mat ((SCREENMAT*)MEM_VRAM)

// Charblocks, 8bpp tiles.
// tile8_mem[y][x] = block y, tile x   ( TILE )
#define tile8_mem  ((CHARBLOCK8*)MEM_VRAM)

#define pal_bg_mem ((COLOR*)MEM_PAL)

#define KEY_MASK   0x03FF
#define KEY_A      0x0001 // Button A
#define KEY_B      0x0002 // Button B
#define KEY_SELECT 0x0004 // Select button
#define KEY_START  0x0008 // Start button
#define KEY_RIGHT  0x0010 // Right D-pad
#define KEY_LEFT   0x0020 // Left D-pad
#define KEY_UP     0x0040 // Up D-pad
#define KEY_DOWN   0x0080 // Down D-pad
#define KEY_R      0x0100 // Shoulder R
#define KEY_L      0x0200 // Shoulder L


#endif // defs_h_INCLUDED
