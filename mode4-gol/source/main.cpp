#include "tonc.h"

#include "page_pic.h"

// Conway's Game of Life on a 120x80 grid using two 1D bool arrays.
// Grid width and height
constexpr int PIXEL_WIDTH = 2;

constexpr int WIDTH = 120;
constexpr int HEIGHT = 80;

u16* front_screen = ((u16*)MEM_VRAM);
u16* back_screen = ((u16*)MEM_VRAM_BACK);

void load_image(){
	for(int ii=0; ii<160; ii++){
		memcpy16(&vid_mem_front[ii*120], &page_picBitmap[ii*240/4], 240/4);
	}
}

// Get cell
u8 get_cell(const u16* src, int gx, int gy) {
	// wrap coordinates to implement toroidal world
	return src[(gy * PIXEL_WIDTH * 120) + (gx)] & 0x00FF;
}

// draw a 2x2 pixel block.
void set_cell(u16* dst, int gx, int gy, u16 color) {
	int ind = (gy*240)+(gx);
	const u16 doubleColor = (color&0xFF)<<8 | (color&0xFF);
	for(int i = 0; i < PIXEL_WIDTH; i++){
		dst[ind+(i*120)] = doubleColor;
	}
}

void clear_image(){
	for(int ii=0; ii<120; ii++){
		for(int jj=0; jj<80; jj++){
			set_cell(front_screen, ii, jj, 0);
		}
	}
}

// Count neighbors. Because x and y will be [1,WIDTH-2] and [1,HEIGHT-2]
// no need to worry about wrap around
// border is always black so edges have fewer neighbors.
int count_neighbors(const u16* src, int x, int y) {
	int count = 0;
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			if (dx == 0 && dy == 0) continue;
			int nx = x + dx;
			int ny = y + dy;
			if (get_cell(src, nx, ny)) ++count;
		}
	}
	return count;
}

void set_pal(){
	pal_bg_mem[1] = RGB15(31,31,31);
}

void set_rand(){
	for(int y = 1; y < HEIGHT-1; y++){
		for(int x = 1; x < WIDTH-1; x++){
			u16 color = (qran() % 4) == 0 ? 1 : 0;
			set_cell(front_screen, x,y, color);
		}
	}
}

void set_glider(){
	set_cell(front_screen, 3, 4, 1);
	set_cell(front_screen, 5, 3, 1);
	set_cell(front_screen, 5, 4, 1);
	set_cell(front_screen, 4, 5, 1);
	set_cell(front_screen, 5, 5, 1);
}

void set_glider_gun(){
	//right half of gun
	set_cell(front_screen, 28, 3, 1);
	set_cell(front_screen, 26, 4, 1);
	set_cell(front_screen, 28, 4, 1);
	set_cell(front_screen, 26, 8, 1);
	set_cell(front_screen, 28, 8, 1);
	set_cell(front_screen, 28, 9, 1);

	set_cell(front_screen, 24, 5, 1);
	set_cell(front_screen, 25, 5, 1);
	set_cell(front_screen, 24, 6, 1);
	set_cell(front_screen, 25, 6, 1);
	set_cell(front_screen, 24, 7, 1);
	set_cell(front_screen, 25, 7, 1);

	//circle layer by layer
	set_cell(front_screen, 16, 5, 1);
	set_cell(front_screen, 17, 5, 1);

	set_cell(front_screen, 15, 6, 1);
	set_cell(front_screen, 19, 6, 1);

	set_cell(front_screen, 14, 7, 1);
	set_cell(front_screen, 20, 7, 1);

	set_cell(front_screen, 14, 8, 1);
	set_cell(front_screen, 18, 8, 1);
	set_cell(front_screen, 20, 8, 1);
	set_cell(front_screen, 21, 8, 1);

	set_cell(front_screen, 14, 9, 1);
	set_cell(front_screen, 20, 9, 1);

	set_cell(front_screen, 15, 10, 1);
	set_cell(front_screen, 19, 10, 1);
	set_cell(front_screen, 16, 11, 1);
	set_cell(front_screen, 17, 11, 1);

	//right square
	set_cell(front_screen, 38, 5, 1);
	set_cell(front_screen, 39, 5, 1);
	set_cell(front_screen, 38, 6, 1);
	set_cell(front_screen, 39, 6, 1);

	//left square
	set_cell(front_screen, 4, 7, 1);
	set_cell(front_screen, 5, 7, 1);
	set_cell(front_screen, 4, 8, 1);
	set_cell(front_screen, 5, 8, 1);
}

void set_toad(){
	set_cell(front_screen, 3, 4, 1);
	set_cell(front_screen, 4, 3, 1);
	set_cell(front_screen, 5, 3, 1);
	set_cell(front_screen, 4, 6, 1);
	set_cell(front_screen, 5, 6, 1);
	set_cell(front_screen, 6, 5, 1);
}

void set_beacon(){
	set_cell(front_screen, 3, 4, 1);
	set_cell(front_screen, 3, 3, 1);
	set_cell(front_screen, 4, 3, 1);
	set_cell(front_screen, 6, 5, 1);
	set_cell(front_screen, 6, 6, 1);
	set_cell(front_screen, 5, 6, 1);
}

// Step one generation from src -> dst (both size WIDTH x HEIGHT)
void step(const u16* src, u16* dst) {
	// iterate full grid; wrapping is handled in the neighbor checks
	for (int y = 1; y < HEIGHT-1; y++) {
		for (int x = 1; x < WIDTH-1; x++) {
			int n = count_neighbors(src, x, y);
			u8 current = get_cell(src, x, y);
			// Live next generation if exactly 3 neighbors, or if 2 neighbors and currently alive
			u8 next = (u8)((n == 3) || (current == 1 && n == 2));
			set_cell(dst, x, y, next);
		}
	}
}

//Load start image, get user input, and set screen for start
void startInput(){
	load_image();

	u32 keyHit = 0;
	while(keyHit == 0){
		qran();
		key_poll();
		keyHit = key_hit(KEY_ANY);
	}

	clear_image();

	switch(keyHit){
		case KEY_A:
			set_glider_gun();
			break;
		case KEY_B:
			set_glider();
			break;
		case KEY_L:
			set_toad();
			break;
		case KEY_R:
			set_beacon();
			break;
		default:
			set_rand();
			break;
	}
}

int main() {
	REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;
	set_pal();

    bool isDisplayingFront;

	while(1){

		isDisplayingFront = true;

		startInput();

		while(1) {
			const u16* src = isDisplayingFront ? front_screen : back_screen;
			u16* dst = isDisplayingFront ? back_screen : front_screen;

			step(src, dst);

			isDisplayingFront = !isDisplayingFront; // alternate arrays per generation

			vid_flip();

			//if start is pressed, back out of simulation
			key_poll();
			if(key_hit(KEY_START) || key_is_down(KEY_START)){break;}
		}

		//reset front image
		if(!isDisplayingFront)vid_flip();
		clear_image();
	}
}
