// Programmed by Piboy314 aka CMDR-Piboy314

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#ifdef __APPLE__
// Defined before OpenGL and GLUT includes to avoid deprecation messages
#define GL_SILENCE_DEPRECATION
#endif

#include <GLUT/glut.h>

#include "../res/img/All_Textures.ppm"
#include "../res/img/sky.ppm"
#include "../res/img/title.ppm"
#include "../res/img/won.ppm"
#include "../res/img/lost.ppm"
#include "../res/img/sprites.ppm"

// Define constants

// Pi stuff
#define PI 3.14159265358975
#define P2 PI / 2
#define P3 3 * PI / 2

// One degree in radians
#define DR 0.0174533

// Map width
#define mapX 8

// Map height
#define mapY 8

// Map cube size
#define mapS 64

// Define variables

// Player position
float px, py, pdx, pdy, pa;

// FPS
float frame1, frame2, fps;


// Button states
typedef struct {
	int w, a, s, d;
}

ButtonKeys;
ButtonKeys Keys;

// Game state
int gameState = 0, timer = 0;

// Fade for the menu screens (Title Loose and Win)
float fade = 0;

// Map

// Map arrays
// Walls
int mapW[] = {
	1,1,1,1,2,2,2,2,
	6,0,0,5,0,0,0,2,
	1,0,0,4,0,2,0,2,
	1,5,5,5,0,0,0,2,
	2,0,0,0,0,0,0,1,
	2,0,0,0,0,1,0,1,
	2,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,
};

// Floors
int mapF[] = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,2,2,2,0,
	0,0,0,0,6,0,2,0,
	0,0,8,0,2,7,6,0,
	0,0,2,0,0,0,0,0,
	0,0,2,0,8,0,0,0,
	0,1,1,1,1,0,8,0,
	0,0,0,0,0,0,0,0,
};

// Ceilings
int mapC[] = {
	4,4,2,4,0,0,0,0,
	4,4,2,4,0,0,0,0,
	4,4,2,4,0,0,0,0,
	4,4,2,4,0,0,0,0,
	4,4,2,4,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};


// All variables needed for a sprite
typedef struct {
	// Static key or enemy
	int type;

	// On or off
	int state;
	
	// Texture
	int map;

	// Position
	float x, y, z;
}

sprite;
sprite sp[4];

// Wall line depth to compare for sprite depth
int depth[120];

float degToRad (float a) {
	return a * M_PI / 180.0;
}

float FixAng (float a) {
	if (a > 359)
		a -= 360;
	
	if (a < 0)
		a += 360;
	
	return a;
}

float distance (ax, ay, bx, by, ang) {
	return cos(degToRad(ang)) * (bx - ax) - sin(degToRad(ang)) * (by - ay);
}

void drawSprite() {
	int x, y, s;

	// Pick up the key
	if (px < sp[0].x + 30 && px > sp[0].x - 30 && py < sp[0].y + 30 && py > sp[0].y - 30)
		sp[0].state = 0;
	
	// Enemy kills
	if (px < sp[3].x + 30 && px > sp[3].x - 30 && py < sp[3].y + 30 && py > sp[3].y - 30)
		gameState = 4;

	// Enemy attack

	// Normal grid position
	int spx = (int)sp[3].x >> 6, spy = (int)sp[3].y >> 6;

	// Normal grid position with offset
	int spx_add = ((int)sp[3].x + 15) >> 6, spy_add = ((int)sp[3].y + 15) >> 6;

	// Normal grid position subtract offset
	int spx_sub = ((int)sp[3].x - 15) >> 6, spy_sub = ((int)sp[3].y - 15) >> 6;

	if (sp[3].x > px && mapW[spy * 8 + spx_sub] == 0)
		sp[3].x -= 0.04 * fps;

	if (sp[3].x < px && mapW[spy * 8 + spx_add] == 0)
		sp[3].x += 0.04 * fps;
	
	if (sp[3].y > py && mapW[spy_sub * 8 + spx] == 0)
		sp[3].y -= 0.04 * fps;
	
	if (sp[3].y < py && mapW[spy_add * 8 + spx] == 0)
		sp[3].y += 0.04 * fps;

	for(s = 0; s < 4; s++) {
		// Temporary float variables
		float sx = sp[s].x - px;
		float sy = sp[s].y - py;
		float sz = sp[s].z;

		// Rotate around origin
		float CS = cos(degToRad(pa)), SN = sin(degToRad(pa));

		float a = sy * CS + sx * SN; 
		float b = sx * CS - sy * SN; 

		sx = a;
		sy = b;

		// Convert to screen coordinates
		sx = (sx * 108.0 / sy) + (120 / 2);
		sy = (sz * 108.0 / sy) + ( 80 / 2);

		// Scale based off distance
		int scale = 32 * 80 / b;
		
		if (scale < 0)
			scale = 0;
		
		if (scale > 120)
			scale = 120;

		// Texture
		float t_x = 0, t_y = 31, t_x_step = 31.5 / (float)scale, t_y_step = 32.0 / (float)scale;

		for(x = sx - scale / 2; x < sx + scale / 2; x++) {
			t_y = 31;

			for(y = 0; y < scale; y++) {
				if(sp[s].state == 1 && x > 0 && x < 120 && b < depth[x]) {
					int pixel=((int)t_y * 32 + (int)t_x) * 3 + (sp[s].map * 32 * 32 * 3);
					int r = sprites[pixel + 0];
					int g = sprites[pixel + 1];
					int b = sprites[pixel + 2];

					// Don't draw it if it's purple
					if(r != 255, g != 0, b != 255) {
						// Draw point
						glPointSize(8);
						glColor3ub(r,g,b);
						glBegin(GL_POINTS);
						
						glVertex2i(x * 8,sy * 8 - y * 8);
						glEnd();
					}

					t_y -= t_y_step;
					
					if(t_y < 0)
						t_y = 0;
				}
			}
			t_x+=t_x_step;
		}
	}
}


// Draw rays and walls
void drawRays2D() {	
	int r, mx, my, mp, dof, side;
	float vx, vy, rx, ry, ra, xo, yo, disV, disH; 

	// Set back rays 30 deg
	ra = FixAng(pa + 30);

	for(r = 0; r < 120; r++) {
		// Vertical and horizontal map texture number
		int vmt = 0, hmt = 0;

		// Vertical
		dof = 0;
		side = 0;
		
		disV = 100000;

		float Tan = tan(degToRad(ra));
		
		// Looking left
		if (cos(degToRad(ra)) > 0.001) {
			rx = (((int)px >> 6) << 6) + 64;
			ry = (px - rx) * Tan + py;

			xo = 64;
			yo = -xo * Tan;
		}

		// Looking right
		else if (cos(degToRad(ra)) <- 0.001) {
			rx = (((int)px >> 6) << 6) - 0.0001;
			ry = (px - rx) * Tan + py;
			
			xo = -64;
			yo = -xo * Tan;
		}

		// Looking up or down, no hit
		else {
			rx = px;
			ry = py;
			
			dof = 8;
		}

		while(dof < 8) 
		{ 
			mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;

			// Hit
			if (mp > 0 && mp < mapX * mapY && mapW[mp] > 0) {
				vmt = mapW[mp] - 1;
				dof = 8;
				
				disV = cos(degToRad(ra)) * (rx - px) - sin(degToRad(ra)) * (ry - py);
			}

			// Check next horizontal
			else {
				rx += xo;
				ry += yo;
				
				dof += 1;
			}
		} 
		vx = rx;
		vy = ry;

		// Horizontal
		dof = 0;
		disH = 100000;

		Tan = 1.0 / Tan;

		// Looking up
		if (sin(degToRad(ra)) > 0.001) {
			ry = (((int)py >> 6) << 6) - 0.0001;
			rx = (py - ry) * Tan + px;

			yo = -64;
			xo = -yo * Tan;
		}

		// Looking down
		else if (sin(degToRad(ra)) <- 0.001) {
			ry = (((int)py >> 6) << 6) + 64;
			rx = (py - ry) * Tan + px;
			
			yo = 64;
			xo = -yo * Tan;
		}

		// Looking straight left or right
		else {
			rx = px;
			ry = py;
			
			dof = 8;
		}

		while (dof < 8) { 
			mx = (int)(rx) >> 6;
			my = (int)(ry) >> 6;
			
			mp = my * mapX + mx;

			// Hit
			if (mp > 0 && mp < mapX * mapY && mapW[mp] > 0) {
				hmt = mapW[mp] - 1;
				dof = 8;
				
				disH = cos(degToRad(ra)) * (rx - px) - sin(degToRad(ra)) * (ry - py);
			}

			// Check next horizontal
			else {
				rx += xo;
				ry += yo;
				
				dof += 1;
			}
		} 

		float shade = 1;

		glColor3f(0, 0.8, 0);

		// Horizontal hit first
		if (disV < disH) {
			hmt = vmt;
			shade = 0.5;

			rx = vx;
			ry = vy;
			disH=disV;
			
			glColor3f(0,0.6,0);
		}

		// Fix the fisheye effect
		int ca = FixAng(pa - ra);
		
		disH = disH * cos(degToRad(ca));

		int lineH = (mapS * 640) / (disH);

		float ty_step=32.0/(float)lineH; 
		float ty_off=0; 

		// Line height and limit
		if (lineH > 640) {
			ty_off = (lineH - 640) / 2.0;
			lineH = 640;
		}

		// Line offset
		int lineOff = 320 - (lineH >> 1);

		// Save line depth
		depth[r] = disH;

		// Draw walls
		int y;

		float ty = ty_off * ty_step; //+hmt*32;
		float tx;

		if (shade == 1) {
			tx = (int)(rx / 2.0) % 32;
			
			if (ra > 180)
				tx = 31 - tx;
		}  
		else {
			tx = (int)(ry / 2.0) % 32;
			
			if (ra > 90 && ra < 270)
				tx = 31 - tx;
		}

		for(y = 0; y < lineH; y++) {
			int pixel = ((int)ty * 32 + (int)tx) * 3 + (hmt * 32 * 32 * 3);
			int red = All_Textures[pixel + 0] * shade;
			int green = All_Textures[pixel + 1] * shade;
			int blue  = All_Textures[pixel + 2] * shade;

			glPointSize(8);
			glColor3ub(red, green, blue);
			glBegin(GL_POINTS);
			
			glVertex2i(r * 8, y + lineOff);

			glEnd();

			ty += ty_step;
		}

		// Draw floors
		for (y = lineOff + lineH; y < 640; y++) {
			float dy = y - (640 / 2.0), deg = degToRad(ra), raFix = cos(degToRad(FixAng(pa - ra)));

			tx = px / 2 + cos(deg) * 158 * 2 * 32 / dy / raFix;
			ty = py / 2 - sin(deg) * 158 * 2 * 32 / dy / raFix;

			int mp = mapF[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
			int pixel = (((int)(ty)&31) * 32 + ((int)(tx)&31)) * 3 + mp * 3;

			int red = All_Textures[pixel + 0] * 0.7;
			int green = All_Textures[pixel + 1] * 0.7;
			int blue = All_Textures[pixel + 2] * 0.7;

			glPointSize(8);
			glColor3ub(red,green,blue);
			glBegin(GL_POINTS);
			
			glVertex2i(r * 8, y);
			
			glEnd();

			// Draw ceiling
			mp = mapC[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
			pixel = (((int)(ty)&31) * 32 + ((int)(tx)&31)) * 3 + mp * 3;

			red = All_Textures[pixel + 0];
			green = All_Textures[pixel + 1];
			blue = All_Textures[pixel + 2];

			if (mp > 0) {
				glPointSize(8);
				glColor3ub(red, green, blue);
				glBegin(GL_POINTS);
				
				glVertex2i(r * 8, 640 - y);
				
				glEnd();
			}
		}

		// Go to the next ray
		ra = FixAng(ra - 0.5);
	}
}

// Draw sky and rotate based off of player position
void drawSky() {
	int x, y;
	
	for(y = 0; y < 40; y++) {
		for(x = 0; x < 120; x++) {
			// Returns 0 to 120 based off player angle
			int xo = (int)pa * 2 - x;

			if (xo < 0) {
				xo += 120;
				xo = xo % 120;
			}

			int pixel = (y * 120 + xo) * 3;
			int r = sky[pixel + 0];
			int g = sky[pixel + 1];
			int b = sky[pixel + 2];

			glPointSize(8);
			glColor3ub(r, g, b);
			glBegin(GL_POINTS);

			glVertex2i(x * 8,y * 8);

			glEnd();
		}	
	}
}


// Draw any full screen (120x80) image
void screen(int v) {
	int x, y;
	int *T;

	if (v == 1)
		T = title;
	
	if (v == 2)
		T = won;
	
	if (v == 3)
		T = lost;
	
	for(y = 0; y < 80; y++) {
		for(x = 0; x < 120; x++) {
			int pixel = (y * 120 + x) * 3;

			int r = T[pixel + 0] * fade;
			int g = T[pixel + 1] * fade;
			int b = T[pixel + 2] * fade;

			glPointSize(8);
			glColor3ub(r, g, b);
			glBegin(GL_POINTS);

			glVertex2i(x * 8,y * 8);

			glEnd();
		}
	}

	if (fade < 1)
		fade += 0.001 * fps;
	
	if (fade > 1)
		fade = 1;
}


// Initialize all variables when the game is started
void init() {
	glClearColor(0.3, 0.3, 0.3, 0);

	px = 150;
	py = 400;
	
	pa = 90;

	// Initialize player position
	pdx = cos(degToRad(pa));
	pdy = -sin(degToRad(pa));

	// Close doors
	mapW[19] = 4;
	mapW[26] = 4;

	// Key
	sp[0].type = 1;
	sp[0].state = 1;
	sp[0].map = 0;

	sp[0].x = 1.5 * 64;
	sp[0].y = 5 * 64;
	sp[0].z = 20;

	// Light 1
	sp[1].type = 2;
	sp[1].state = 1;
	sp[1].map = 1;
	
	sp[1].x = 1.5 * 64;
	sp[1].y = 4.5 * 64;
	sp[1].z = 0;

	// Light 2
	sp[2].type = 2;
	sp[2].state = 1;
	sp[2].map = 1;
	
	sp[2].x = 3.5 * 64;
	sp[2].y = 4.5 * 64;
	sp[2].z = 0;

	// Enemy
	sp[3].type = 3;
	sp[3].state = 1;
	sp[3].map = 2;
	sp[3].x = 2.5 * 64;
	sp[3].y = 2 * 64;
	sp[3].z = 20;
}


void display() {
	// FPS
	frame2 = glutGet(GLUT_ELAPSED_TIME);
	fps = (frame2 - frame1);
	frame1 = glutGet(GLUT_ELAPSED_TIME); 

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// Initialize game
	if(gameState == 0) {
		init();

		fade = 0;
		timer = 0;
		gameState = 1;
	}

	// Start screen
	if (gameState==1) {
		screen(1);
		timer += 1 * fps;

		if (timer > 2000) {
			fade = 0;
			timer = 0;

			gameState = 2;
		}
	}

	// Main loop
	if(gameState == 2) {
		// Buttons
		if (Keys.a == 1) {
			pa += 0.2 * fps;
			pa = FixAng(pa);
			
			pdx = cos(degToRad(pa));
			pdy = -sin(degToRad(pa));
		}

		if (Keys.d == 1) {
			pa -= 0.2 * fps;
			pa = FixAng(pa);
			
			pdx = cos(degToRad(pa));
			pdy = -sin(degToRad(pa));
		} 

		// X offset to check map
		int xo = 0;
		
		if (pdx < 0)
			xo = -20;
		
		else
			xo = 20;

		// Y offset to check map
		int yo = 0;
		
		if (pdy < 0)
			yo = -20;
		
		else
			yo = 20;

		// X position and offset
		int ipx = px / 64.0, ipx_add_xo = (px + xo) / 64.0, ipx_sub_xo = (px - xo) / 64.0;

		// Y position and offset
		int ipy = py / 64.0, ipy_add_yo = (py + yo) / 64.0, ipy_sub_yo = (py - yo) / 64.0;

		// Move forward
		if (Keys.w == 1) {  
			if (mapW[ipy * mapX + ipx_add_xo] == 0)
				px += pdx * 0.2 * fps;
			
			if (mapW[ipy_add_yo * mapX + ipx] == 0)
				py += pdy * 0.2 * fps;
		}

		// Move backward
		if (Keys.s == 1) {
			if (mapW[ipy * mapX + ipx_sub_xo] == 0)
				px -= pdx * 0.2 * fps;
			
			if (mapW[ipy_sub_yo * mapX + ipx] == 0)
				py -= pdy * 0.2 * fps;
		} 

		drawSky();
		drawRays2D();
		drawSprite();

		// Hit block 1, player wins!
		if ((int)px >> 6 == 1 && (int)py >> 6 == 1) {
			fade = 0;
			timer = 0;
			gameState = 3;
		}
	}

	// Win screen
	if (gameState == 3) {
		screen(2);
		timer += 1 * fps;
		
		if (timer > 2000) {
			fade = 0;
			timer = 0;
			gameState = 0;
		}
	}

	// Loose screen
	if (gameState == 4) {
		screen(3);
		timer += 1 * fps;
		
		if (timer > 2000) {
			fade = 0;
			timer = 0;
			gameState = 0;
		}
	}

	glutPostRedisplay();
	glutSwapBuffers();  
}

// Key hit down
void ButtonDown(unsigned char key, int x, int y) {
	if(key == 'a')
		Keys.a = 1;
	
	if(key == 'd')
		Keys.d = 1;
	
	if(key == 'w')
		Keys.w=1;
	
	if(key == 's')
		Keys.s=1;

	// Open door
	if(key == 'e' && sp[0].state == 0) { 
		int xo = 0;
		
		if (pdx < 0)
			xo = -25;
		
		else
			xo = 25;
		
		int yo = 0;
		
		if (pdy < 0)
			yo = -25;
		
		else
			yo = 25;
		 
		int ipx = px / 64.0, ipx_add_xo = (px + xo) / 64.0;
		int ipy = py / 64.0, ipy_add_yo = (py + yo) / 64.0;

		if (mapW[ipy_add_yo * mapX + ipx_add_xo] == 4)
			mapW[ipy_add_yo * mapX + ipx_add_xo] = 0;
	}

	glutPostRedisplay();
}

// Key let go of
void ButtonUp(unsigned char key, int x, int y) {
	if(key == 'a')
		Keys.a = 0;
	
	if(key == 'd')
		Keys.d = 0;
	
	if(key == 'w')
		Keys.w = 0;
	
	if(key == 's')
		Keys.s = 0;
	
	glutPostRedisplay();
}

// When the window is resized, snap back to 960, 640
void resize(int w, int h) {
	glutReshapeWindow(960, 640);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(960 ,640);
	glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH) / 2 - 960 / 2, glutGet(GLUT_SCREEN_HEIGHT) / 2 - 640 / 2);

	glutCreateWindow("The Ray Game!");
	gluOrtho2D(0, 960, 640, 0);

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	
	glutKeyboardFunc(ButtonDown);
	glutKeyboardUpFunc(ButtonUp);

	glutMainLoop();
}
