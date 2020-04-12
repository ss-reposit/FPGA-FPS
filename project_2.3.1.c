/***************** Double framed, added all features ***********/
/***************** 160 * 120 ***************/
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#define nScreenWidth 160 
#define nScreenHeight 120
#define nMapHeight 16
#define nMapWidth 16
#define nBlobWidth 4
#define nBlobHeight 4
#define nFireBallWidth 4
#define nFireBallHeight 4
#define nSpriteArea 16
#define nListObjectMaxSize 10
#define white 0xFFFF
#define black 0x0000
#define yellow 0xFF20
#define green 0x0C41
#define orange 0xF380
typedef struct sSprite{
	 short int nHeight;
	 short int nWidth;
	 short int* nColourArray;
    
    short int (*SampleColour)(const struct sSprite* s, float fSampleX, float fSampleY);
}sSprite;

typedef struct sObject{
	double x;
	double y;
	double vx;
	double vy;
	bool bRemove;
	sSprite* sprite;
}sObject;


short int nBlobSpriteColours[nSpriteArea] = {green, green, green, green, 
												black, yellow, yellow, black,
												black, yellow, yellow, black,
												green, green, green, green
												};
    
short int nFireBallSpriteColours[nSpriteArea] = {
												black, black, black, black, 
												black, orange, yellow, black,
												black, yellow, orange, black,
												black, black, black, black
												};


sSprite* spriteFireBall;
sSprite* spriteBlob;


float* fDepthBuffer = NULL;

volatile int pixel_buffer_start; // global variable
volatile int* swPtr = (int*) 0xFF200040;
volatile int* ledPtr = (int*) 0xFF200000;
volatile int* keyEdgePtr = (int*) 0xFF20005C;

short int switchPower();
short int wait_for_vsync();
void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x1, int y1, int x2, int y2, short int line_color);
void swap(int*, int*);
void draw_rect(int x, int y, int width, int length, short int color);
short int keyPushed();
short int SampleColour(const struct sSprite* s, float fSampleX, float fSampleY);
int main(){
	float fPlayerX = 4.0f;
	float fPlayerY = 4.0f;
	float fPlayerA = 0.0f; //angle
	float fFOV = 3.14159 / 4.0; //fov is 90 degrees
	float fDepth = 16.0f;
	float fSpeed = 1.0f;
	short int nResolutionX = 2;
	short int nResolutionY = 2;
	float fRotateSpeed = 0.3f;
	float fDistanceResolution = 0.5f;
	
	spriteFireBall = (sSprite*) malloc(sizeof(sSprite));
    spriteBlob = (sSprite*) malloc(sizeof(sSprite));
    
    short int* nColourFireBall = (short int*) malloc(nSpriteArea*sizeof(short int));
    short int* nColourBlob = (short int*) malloc(nSpriteArea*sizeof(short int));
	
	
	for(int i = 0; i < nSpriteArea; i++){ //inti the sprite colours
        *(nColourFireBall + i) = nFireBallSpriteColours[i];
        *(nColourBlob + i) = nBlobSpriteColours[i];
    }
    
    spriteFireBall->nHeight = (short int) nFireBallHeight;
    spriteFireBall->nWidth = (short int) nFireBallWidth;
    spriteFireBall->nColourArray = nColourFireBall;
    spriteFireBall->SampleColour = SampleColour;
    
    spriteBlob->nHeight = (short int) nBlobHeight;
    spriteBlob->nWidth = (short int) nBlobWidth;
    spriteBlob->nColourArray = nColourBlob;
    spriteBlob->SampleColour = SampleColour;
	
	
	sObject listObjects[nListObjectMaxSize] ={{8.5f, 8.5f, 0.5f, 0.5f, false, spriteBlob},
											{6.5f, 2.5f, 0.5f, 0.5f, false, spriteBlob},
											{10.5f, 4.5f, 0.5f, 0.5f, false, spriteBlob}											
											};
												
	fDepthBuffer = malloc(sizeof(float)*nScreenWidth); // one for every line
	//Need a mad, 2d array # = wall, . is empty space
	//16 by 16 maps (map is a float pointer to char)
	const char* map[nMapHeight] = 
	{	//x------------->
		"##########.....#",
		"#..............#",
		"#.........######",
		"#..............#",
		"#.......##.....#",
		"#.......##.....#",
		"#..............#",
		"####...........#",
		"##.............#",
		"#.......###..###",
		"#..............#",
		"#......#.......#",
		"#......#.......#",
		"#..............#",
		"#..............#",
		"################"
	};
	
	//set the previous buffer to -1; because that is our default value
	//game loop
	//Need to clear the screen so that its a blank canvas
	volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	
	short int key_pressed = -1;
	short int led_bits = 0;
	// let 0 = rotate right;  1 = move forwards; 2 =  move backwards; 3 = rotate left
	clear_screen(); //clear both buffers just in case
	while(1){
		//clear_screen();
		//controls this is where the interrupts need to be handled
		//do nothing, either first time looping or no key pressed or couldn't poll in time 
		switch (key_pressed) {
            case 0: led_bits = 0b1;
                break;
            case 1: led_bits = 0b10;
                break;
            case 2: led_bits = 0b100;
                break;
            case 3: led_bits = 0b1000;
                break;
			case 4: led_bits = 0b10000;
				break;
			case 5: led_bits = 0b100000;
				break;
			case 6: led_bits = 0b1000000;
				break;
            default: led_bits = 0b0;
        }
        (*ledPtr) = led_bits;
		if(key_pressed == 0){ //rotate right
			fPlayerA += fRotateSpeed;
		}
		else if(key_pressed == 1){ //move forward
			fPlayerX += sinf(fPlayerA) * fSpeed;
			fPlayerY += cosf(fPlayerA) * fSpeed;
			if (map[(int)fPlayerY][(int)fPlayerX] == '#') //collision detection, if in wall move out
			{
				fPlayerX -= sinf(fPlayerA) * fSpeed;
				fPlayerY -= cosf(fPlayerA) * fSpeed;
			}
		}
		else if(key_pressed == 2){ //move backwards
			fPlayerX -= sinf(fPlayerA) * fSpeed;
			fPlayerY -= cosf(fPlayerA) * fSpeed;
			if (map[(int)fPlayerY][(int)fPlayerX] == '#') //collision detection, if in wall move forward
			{
				fPlayerX += sinf(fPlayerA) * fSpeed;
				fPlayerY += cosf(fPlayerA) * fSpeed;
			}
		}
		else if(key_pressed == 3){ //rotate left
			fPlayerA -= fRotateSpeed;
		}
		else if(key_pressed == 4){ //strafe right
			fPlayerX += cosf(fPlayerA) * fSpeed;
			fPlayerY -= sinf(fPlayerA) * fSpeed;
			if (map[(int)fPlayerY][(int)fPlayerX] == '#')
			{
				fPlayerX -= cosf(fPlayerA) * fSpeed;
				fPlayerY += sinf(fPlayerA) * fSpeed;
			}
		}
		else if(key_pressed == 5){ //strafe left
			fPlayerX -= cosf(fPlayerA) * fSpeed;
			fPlayerY += sinf(fPlayerA) * fSpeed;
			if (map[(int)fPlayerY][(int)fPlayerX] == '#')
			{
				fPlayerX += cosf(fPlayerA) * fSpeed;
				fPlayerY -= sinf(fPlayerA) * fSpeed;
			}
		}
		else if(key_pressed == 6){ //shot release
			//printf("found the key was pressed");
			int i = 0;
			while(i < nListObjectMaxSize && listObjects[i].bRemove == false) i++;//find back
			if(i != nListObjectMaxSize){
				sObject o;
				o.x = fPlayerX;
				o.y = fPlayerY;
				
				float fNoise = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.1f;
				o.vx = sinf(fPlayerA + fNoise) * 1.0f;
				o.vy = cosf(fPlayerA + fNoise) * 1.0f;
				o.bRemove = false;
				o.sprite = spriteFireBall;
				//printf("set object vx: %f, vy: %f", (double ) o.vx, (double) o.vy);
				listObjects[i] = o;
				
			}
			(*keyEdgePtr) = 0b01; //reset edge capture
		}
		else{ //do nothing because key_pressed = -1;
		}
		for(int x = 0;  x < nScreenWidth; x+=nResolutionX){ //go through each coloumn on the screen
			//ray tracing algorithm
			float fRayAngle = (fPlayerA - fFOV/2.0f) + ((float)x / (float)nScreenWidth) * fFOV; //chop up FOV based into scrren witdh size differentials
			
			//get distance of player to wall
			float fDistanceToWall = 0.0f;
			//get distance by slowing incrementing a differential length 
			//until you hit a wall 
			bool bHitWall = false;
			//need to unit vector that points in direction of this ray
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);
			
			//to prevent infinite loop need to limit the check distance
			
			while(!bHitWall && fDistanceToWall < fDepth){
				fDistanceToWall+= fDistanceResolution; //differntial length is 0.1f
				
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall); //x proj
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall); //y proj
				
				//check out of bounds because need value
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;
					fDistanceToWall = fDepth; //dont render it will just be black
				}
				else{ //within bounds, now check if wall is hit
					if(map[nTestY][nTestX] == '#'){ //vector of 4 points representing the 4 corners and then storing in it an array of 2 //then display closest 2
						bHitWall = true;
					}
				}
			}
			//found distance to wall for this particular ray
			//now need to calculate height of wall to create perspective
			fDepthBuffer[x] = fDistanceToWall;//depth of each column in now recorded
			

			int nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float)fDistanceToWall); //distance large cieling large
			int nFloor = nScreenHeight - nCeiling;
			
			
			
			//found all information for this ray
			//now need to draw in a differential line of pixels into the buffer
			short int nShadeWall;
			short int nShadeFloor;
			if (fDistanceToWall <= fDepth / 4.0f)			nShadeWall = 0xFFFF;	// Very close	
			else if (fDistanceToWall < fDepth / 3.0f)		nShadeWall = 0xA534;
			else if (fDistanceToWall < fDepth / 2.0f)		nShadeWall = 0x6B4D;
			else if (fDistanceToWall < fDepth)				nShadeWall = 0x39C7;
			else											nShadeWall = 0x0000;
			for(int y = 0; y < nScreenHeight; y+=nResolutionY){
				if(y < nCeiling){ //this pixel is ceiling
					//call plot pixel to fill in appropriate location
					//in the pixel buffer
					plot_pixel(x, y, black);
				}
				else if(y > nCeiling && y <= nFloor){
					//plot pixel for wall colour
						plot_pixel(x, y, nShadeWall);
				}
				else{
					//plot the floor
					float b = 1.0f - (((float)y -nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShadeFloor = 0xD8A2;
					else if (b < 0.5)	nShadeFloor = 0xB8A2;
					else if (b < 0.75)	nShadeFloor = 0x8861;
					else if (b < 0.9)	nShadeFloor = 0x5841;
					else				nShadeFloor = 0x0000;
					plot_pixel(x, y, nShadeFloor);
				}
			}	
		}
		
		for(int nObjectIndex = 0; nObjectIndex < nListObjectMaxSize; nObjectIndex++){
			if(listObjects[nObjectIndex].bRemove == true) continue;
			struct sObject* object = &listObjects[nObjectIndex];
			
			//update object physics
			
			/*printf("found object index: %d\n", nObjectIndex);
			printf("found object x: %f\n",  (double) object->x );
			printf("found object y: %f\n", (double) object->y);
			printf("found object vx: %f\n", (double) object->vx);
			printf("found object vy: %f\n", (double) object->vy);*/
			
			object->x = object->x + object->vx;
			object->y = object->y + object->vy;
				
			//set remove if in wall //check not negative etc 
			if(map[(int) object->y][(int) object->x] == '#'){
				if(object->sprite == spriteBlob){
					object->x = object->x - object->vx; //undo it first 
					object->y = object->y - object->vy;
					
					object->vx = (-1) * object->vx; //then redo it
					object->vy = (-1) * object->vy;
				}
				else{
					object->bRemove = true;
					//printf("removed");
					listObjects[nObjectIndex].bRemove = true; //use this as the "deleted" flag
					continue;
				}
			}
			
			float fVecX = object->x - fPlayerX;
			float fVecY = object->y - fPlayerY;
			float fDistanceFromPlayer = sqrtf(fVecX*fVecX + fVecY*fVecY);
			
			float fEyeX = sinf(fPlayerA);
			float fEyeY = cosf(fPlayerA);

			// Calculate angle between lamp and players feet, and players looking direction
			// to determine if the lamp is in the players field of view
			float fObjectAngle = atan2f(fEyeY, fEyeX) - atan2f(fVecY, fVecX);
			if (fObjectAngle < -3.14159f)
				fObjectAngle += 2.0f * 3.14159f;
			if (fObjectAngle > 3.14159f)
				fObjectAngle -= 2.0f * 3.14159f;
			
			bool bInPlayerFOV = fabs(fObjectAngle) < fFOV / 2.0f; 
			if (bInPlayerFOV && fDistanceFromPlayer >= 0.5f && fDistanceFromPlayer < fDepth){ //if too close dont draw, if top far dont draw
				
				if (object->sprite != spriteBlob){ //decrease size of fireballs
					fDistanceFromPlayer *=4;
				}
				if(fDistanceFromPlayer < 2.0) fDistanceFromPlayer = 2.0; //dont want negative Celiing
				float fObjectCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceFromPlayer);
				if (object->sprite != spriteBlob){
					fDistanceFromPlayer /=4;
				}
				float fObjectFloor = nScreenHeight - fObjectCeiling;
				float fObjectHeight = (fObjectFloor - fObjectCeiling) ;
				float fObjectAspectRatio = (float)object->sprite->nHeight / (float)object->sprite->nWidth;
				float fObjectWidth = (fObjectHeight / fObjectAspectRatio);
				float fMiddleOfObject = (0.5f * (fObjectAngle / (fFOV / 2.0f)) + 0.5f) * (float)nScreenWidth;	
				
				//Draw Object
				
				for (float lx = 0; lx < fObjectWidth; lx+=nResolutionX)
				{
					for (float ly = 0; ly < fObjectHeight; ly+=nResolutionY)
					{
						float fSampleX = lx / fObjectWidth;
						float fSampleY = ly / fObjectHeight;
						int c = object->sprite->SampleColour( object->sprite, fSampleX, fSampleY);
						int nObjectColumn = (int)(fMiddleOfObject + lx - (fObjectWidth / 2.0f));
						int nObjectRow = (int) (fObjectCeiling + ly);
						if(nObjectColumn % nResolutionX != 0) nObjectColumn = nObjectColumn - (nObjectColumn % nResolutionX); //shift so no clear neeeded!
						if( fDepthBuffer[nObjectColumn] < fDistanceFromPlayer) break; //save some computation 
						
						if(nObjectRow  % nResolutionY != 0) nObjectRow = nObjectRow - (nObjectRow  % nResolutionY); 	
						if (nObjectColumn >= 0 && nObjectColumn < nScreenWidth){
							//printf("fObjectCeiling: %f, ly: %f: colour: %d", fObjectCeiling, ly, c);
							if(c != black && nObjectRow >= 0 && nObjectRow < nScreenHeight){ //empty space
								plot_pixel(nObjectColumn, nObjectRow, c);
								fDepthBuffer[nObjectColumn] = fDistanceFromPlayer;
							}
						}			
					}
				}
			}
		}
		
		
		//this where I draw
		//this is also where i swap buffer and pointer etc
		key_pressed = wait_for_vsync(); // swap front and back buffers on VGA vertical sync
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
	}
	
	
	free(nColourFireBall);
    free(nColourBlob);
    free(spriteFireBall);
    free(spriteBlob);
	free(fDepthBuffer);
	return 0;
}


short int SampleColour(const struct sSprite* s, float fSampleX, float fSampleY){
	int sx = (int)(fSampleX * (float)s->nWidth);
	int sy = (int)(fSampleY * (float)s->nHeight);
	//printf("s->nWidth: %d s->nHeight: %d sx: %d sy: %d",s->nWidth, s->nHeight, sx, sy);
	
	if(sx >= s->nWidth || sx < 0 || sy >= s->nHeight || sy < 0){
		printf("ERROR: Tried to fetch sprite colour out of bounds");
		return -1;
	}
	else
		return *(s->nColourArray + sy*s->nWidth + sx);
}
short int switchPower() {
    short int value = -1;
    if ((*swPtr & (0b01)) != 0) value = 0;
    if ((*swPtr & (0b10)) != 0) value = 1;
    if ((*swPtr & (0b100)) != 0) value = 2;
    if ((*swPtr & (0b1000)) != 0) value = 3;
    if ((*swPtr & (0b10000)) != 0) value = 4;
    if ((*swPtr & (0b100000)) != 0) value = 5;
	
	if(value == -1) return keyPushed(); //if not moving then check for shoot
    return value;
}
// The helper function
// Reads the edge capture reg of key 0 and returns if its 0 or 1

short int keyPushed() {
    short int value = -1; //Initialized it to a random number for testing
	if (((*keyEdgePtr) & (0b01)) == 1){
		value = 6;
	}
    return value;
}

void plot_pixel(int x, int y, short int line_color)
{
    if(x >= nScreenWidth || x < 0 || y >= nScreenHeight || y < 0)
		printf("ERROR: Tried to draw out of bounds");
	else
		*(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void swap(int* a, int* b){
	int temp = *a;
	*a = *b;
	*b = temp;
}
short int wait_for_vsync(){
	int status;
	volatile int * ctrl_reg = (int *)0xFF203020;
	*(ctrl_reg) = 1;
	status = *(ctrl_reg + 3);
	short int switch_pressed = -1;
	while((status & 0x1) != 0){ //doing polling while waiting for vsync this way save some cycles but have some input lag
		status = *(ctrl_reg + 3);
		switch_pressed = switchPower();   
	}
	return switch_pressed;
}

void clear_screen(){
	//memset((short int*) pixel_buffer_start, 0, 245760); //memset O(n) complexity, much faster than double loop 
	//short int* addr = (short int*) pixel_buffer_start;
	//for(int i = 0; i < 240; i++){ //320*240*2/640 = 240
	//	memset(addr, 0, 640);
	//	addr = addr + (short int) ((0x280 + 0x180)/2) ; //divide by 2 because short int pointer integer artmetic is size 2 		
	//}	
	
	//fastest possible way to clear the screen
	if(pixel_buffer_start == 0xC8000000){
		memset((short int*) 0xC8000000, 0, 320);
		memset((short int*) 0xC8000400, 0, 320);
		memset((short int*) 0xC8000800, 0, 320);
		memset((short int*) 0xC8000C00, 0, 320);
		memset((short int*) 0xC8001000, 0, 320);
		memset((short int*) 0xC8001400, 0, 320);
		memset((short int*) 0xC8001800, 0, 320);
		memset((short int*) 0xC8001C00, 0, 320);
		memset((short int*) 0xC8002000, 0, 320);
		memset((short int*) 0xC8002400, 0, 320);
		memset((short int*) 0xC8002800, 0, 320);
		memset((short int*) 0xC8002C00, 0, 320);
		memset((short int*) 0xC8003000, 0, 320);
		memset((short int*) 0xC8003400, 0, 320);
		memset((short int*) 0xC8003800, 0, 320);
		memset((short int*) 0xC8003C00, 0, 320);
		memset((short int*) 0xC8004000, 0, 320);
		memset((short int*) 0xC8004400, 0, 320);
		memset((short int*) 0xC8004800, 0, 320);
		memset((short int*) 0xC8004C00, 0, 320);
		memset((short int*) 0xC8005000, 0, 320);
		memset((short int*) 0xC8005400, 0, 320);
		memset((short int*) 0xC8005800, 0, 320);
		memset((short int*) 0xC8005C00, 0, 320);
		memset((short int*) 0xC8006000, 0, 320);
		memset((short int*) 0xC8006400, 0, 320);
		memset((short int*) 0xC8006800, 0, 320);
		memset((short int*) 0xC8006C00, 0, 320);
		memset((short int*) 0xC8007000, 0, 320);
		memset((short int*) 0xC8007400, 0, 320);
		memset((short int*) 0xC8007800, 0, 320);
		memset((short int*) 0xC8007C00, 0, 320);
		memset((short int*) 0xC8008000, 0, 320);
		memset((short int*) 0xC8008400, 0, 320);
		memset((short int*) 0xC8008800, 0, 320);
		memset((short int*) 0xC8008C00, 0, 320);
		memset((short int*) 0xC8009000, 0, 320);
		memset((short int*) 0xC8009400, 0, 320);
		memset((short int*) 0xC8009800, 0, 320);
		memset((short int*) 0xC8009C00, 0, 320);
		memset((short int*) 0xC800A000, 0, 320);
		memset((short int*) 0xC800A400, 0, 320);
		memset((short int*) 0xC800A800, 0, 320);
		memset((short int*) 0xC800AC00, 0, 320);
		memset((short int*) 0xC800B000, 0, 320);
		memset((short int*) 0xC800B400, 0, 320);
		memset((short int*) 0xC800B800, 0, 320);
		memset((short int*) 0xC800BC00, 0, 320);
		memset((short int*) 0xC800C000, 0, 320);
		memset((short int*) 0xC800C400, 0, 320);
		memset((short int*) 0xC800C800, 0, 320);
		memset((short int*) 0xC800CC00, 0, 320);
		memset((short int*) 0xC800D000, 0, 320);
		memset((short int*) 0xC800D400, 0, 320);
		memset((short int*) 0xC800D800, 0, 320);
		memset((short int*) 0xC800DC00, 0, 320);
		memset((short int*) 0xC800E000, 0, 320);
		memset((short int*) 0xC800E400, 0, 320);
		memset((short int*) 0xC800E800, 0, 320);
		memset((short int*) 0xC800EC00, 0, 320);
		memset((short int*) 0xC800F000, 0, 320);
		memset((short int*) 0xC800F400, 0, 320);
		memset((short int*) 0xC800F800, 0, 320);
		memset((short int*) 0xC800FC00, 0, 320);
		memset((short int*) 0xC8010000, 0, 320);
		memset((short int*) 0xC8010400, 0, 320);
		memset((short int*) 0xC8010800, 0, 320);
		memset((short int*) 0xC8010C00, 0, 320);
		memset((short int*) 0xC8011000, 0, 320);
		memset((short int*) 0xC8011400, 0, 320);
		memset((short int*) 0xC8011800, 0, 320);
		memset((short int*) 0xC8011C00, 0, 320);
		memset((short int*) 0xC8012000, 0, 320);
		memset((short int*) 0xC8012400, 0, 320);
		memset((short int*) 0xC8012800, 0, 320);
		memset((short int*) 0xC8012C00, 0, 320);
		memset((short int*) 0xC8013000, 0, 320);
		memset((short int*) 0xC8013400, 0, 320);
		memset((short int*) 0xC8013800, 0, 320);
		memset((short int*) 0xC8013C00, 0, 320);
		memset((short int*) 0xC8014000, 0, 320);
		memset((short int*) 0xC8014400, 0, 320);
		memset((short int*) 0xC8014800, 0, 320);
		memset((short int*) 0xC8014C00, 0, 320);
		memset((short int*) 0xC8015000, 0, 320);
		memset((short int*) 0xC8015400, 0, 320);
		memset((short int*) 0xC8015800, 0, 320);
		memset((short int*) 0xC8015C00, 0, 320);
		memset((short int*) 0xC8016000, 0, 320);
		memset((short int*) 0xC8016400, 0, 320);
		memset((short int*) 0xC8016800, 0, 320);
		memset((short int*) 0xC8016C00, 0, 320);
		memset((short int*) 0xC8017000, 0, 320);
		memset((short int*) 0xC8017400, 0, 320);
		memset((short int*) 0xC8017800, 0, 320);
		memset((short int*) 0xC8017C00, 0, 320);
		memset((short int*) 0xC8018000, 0, 320);
		memset((short int*) 0xC8018400, 0, 320);
		memset((short int*) 0xC8018800, 0, 320);
		memset((short int*) 0xC8018C00, 0, 320);
		memset((short int*) 0xC8019000, 0, 320);
		memset((short int*) 0xC8019400, 0, 320);
		memset((short int*) 0xC8019800, 0, 320);
		memset((short int*) 0xC8019C00, 0, 320);
		memset((short int*) 0xC801A000, 0, 320);
		memset((short int*) 0xC801A400, 0, 320);
		memset((short int*) 0xC801A800, 0, 320);
		memset((short int*) 0xC801AC00, 0, 320);
		memset((short int*) 0xC801B000, 0, 320);
		memset((short int*) 0xC801B400, 0, 320);
		memset((short int*) 0xC801B800, 0, 320);
		memset((short int*) 0xC801BC00, 0, 320);
		memset((short int*) 0xC801C000, 0, 320);
		memset((short int*) 0xC801C400, 0, 320);
		memset((short int*) 0xC801C800, 0, 320);
		memset((short int*) 0xC801CC00, 0, 320);
		memset((short int*) 0xC801D000, 0, 320);
		memset((short int*) 0xC801D400, 0, 320);
		memset((short int*) 0xC801D800, 0, 320);
		memset((short int*) 0xC801DC00, 0, 320);
	}
	else{
		memset((short int*) 0xC0000000, 0, 320);
		memset((short int*) 0xC0000400, 0, 320);
		memset((short int*) 0xC0000800, 0, 320);
		memset((short int*) 0xC0000C00, 0, 320);
		memset((short int*) 0xC0001000, 0, 320);
		memset((short int*) 0xC0001400, 0, 320);
		memset((short int*) 0xC0001800, 0, 320);
		memset((short int*) 0xC0001C00, 0, 320);
		memset((short int*) 0xC0002000, 0, 320);
		memset((short int*) 0xC0002400, 0, 320);
		memset((short int*) 0xC0002800, 0, 320);
		memset((short int*) 0xC0002C00, 0, 320);
		memset((short int*) 0xC0003000, 0, 320);
		memset((short int*) 0xC0003400, 0, 320);
		memset((short int*) 0xC0003800, 0, 320);
		memset((short int*) 0xC0003C00, 0, 320);
		memset((short int*) 0xC0004000, 0, 320);
		memset((short int*) 0xC0004400, 0, 320);
		memset((short int*) 0xC0004800, 0, 320);
		memset((short int*) 0xC0004C00, 0, 320);
		memset((short int*) 0xC0005000, 0, 320);
		memset((short int*) 0xC0005400, 0, 320);
		memset((short int*) 0xC0005800, 0, 320);
		memset((short int*) 0xC0005C00, 0, 320);
		memset((short int*) 0xC0006000, 0, 320);
		memset((short int*) 0xC0006400, 0, 320);
		memset((short int*) 0xC0006800, 0, 320);
		memset((short int*) 0xC0006C00, 0, 320);
		memset((short int*) 0xC0007000, 0, 320);
		memset((short int*) 0xC0007400, 0, 320);
		memset((short int*) 0xC0007800, 0, 320);
		memset((short int*) 0xC0007C00, 0, 320);
		memset((short int*) 0xC0008000, 0, 320);
		memset((short int*) 0xC0008400, 0, 320);
		memset((short int*) 0xC0008800, 0, 320);
		memset((short int*) 0xC0008C00, 0, 320);
		memset((short int*) 0xC0009000, 0, 320);
		memset((short int*) 0xC0009400, 0, 320);
		memset((short int*) 0xC0009800, 0, 320);
		memset((short int*) 0xC0009C00, 0, 320);
		memset((short int*) 0xC000A000, 0, 320);
		memset((short int*) 0xC000A400, 0, 320);
		memset((short int*) 0xC000A800, 0, 320);
		memset((short int*) 0xC000AC00, 0, 320);
		memset((short int*) 0xC000B000, 0, 320);
		memset((short int*) 0xC000B400, 0, 320);
		memset((short int*) 0xC000B800, 0, 320);
		memset((short int*) 0xC000BC00, 0, 320);
		memset((short int*) 0xC000C000, 0, 320);
		memset((short int*) 0xC000C400, 0, 320);
		memset((short int*) 0xC000C800, 0, 320);
		memset((short int*) 0xC000CC00, 0, 320);
		memset((short int*) 0xC000D000, 0, 320);
		memset((short int*) 0xC000D400, 0, 320);
		memset((short int*) 0xC000D800, 0, 320);
		memset((short int*) 0xC000DC00, 0, 320);
		memset((short int*) 0xC000E000, 0, 320);
		memset((short int*) 0xC000E400, 0, 320);
		memset((short int*) 0xC000E800, 0, 320);
		memset((short int*) 0xC000EC00, 0, 320);
		memset((short int*) 0xC000F000, 0, 320);
		memset((short int*) 0xC000F400, 0, 320);
		memset((short int*) 0xC000F800, 0, 320);
		memset((short int*) 0xC000FC00, 0, 320);
		memset((short int*) 0xC0010000, 0, 320);
		memset((short int*) 0xC0010400, 0, 320);
		memset((short int*) 0xC0010800, 0, 320);
		memset((short int*) 0xC0010C00, 0, 320);
		memset((short int*) 0xC0011000, 0, 320);
		memset((short int*) 0xC0011400, 0, 320);
		memset((short int*) 0xC0011800, 0, 320);
		memset((short int*) 0xC0011C00, 0, 320);
		memset((short int*) 0xC0012000, 0, 320);
		memset((short int*) 0xC0012400, 0, 320);
		memset((short int*) 0xC0012800, 0, 320);
		memset((short int*) 0xC0012C00, 0, 320);
		memset((short int*) 0xC0013000, 0, 320);
		memset((short int*) 0xC0013400, 0, 320);
		memset((short int*) 0xC0013800, 0, 320);
		memset((short int*) 0xC0013C00, 0, 320);
		memset((short int*) 0xC0014000, 0, 320);
		memset((short int*) 0xC0014400, 0, 320);
		memset((short int*) 0xC0014800, 0, 320);
		memset((short int*) 0xC0014C00, 0, 320);
		memset((short int*) 0xC0015000, 0, 320);
		memset((short int*) 0xC0015400, 0, 320);
		memset((short int*) 0xC0015800, 0, 320);
		memset((short int*) 0xC0015C00, 0, 320);
		memset((short int*) 0xC0016000, 0, 320);
		memset((short int*) 0xC0016400, 0, 320);
		memset((short int*) 0xC0016800, 0, 320);
		memset((short int*) 0xC0016C00, 0, 320);
		memset((short int*) 0xC0017000, 0, 320);
		memset((short int*) 0xC0017400, 0, 320);
		memset((short int*) 0xC0017800, 0, 320);
		memset((short int*) 0xC0017C00, 0, 320);
		memset((short int*) 0xC0018000, 0, 320);
		memset((short int*) 0xC0018400, 0, 320);
		memset((short int*) 0xC0018800, 0, 320);
		memset((short int*) 0xC0018C00, 0, 320);
		memset((short int*) 0xC0019000, 0, 320);
		memset((short int*) 0xC0019400, 0, 320);
		memset((short int*) 0xC0019800, 0, 320);
		memset((short int*) 0xC0019C00, 0, 320);
		memset((short int*) 0xC001A000, 0, 320);
		memset((short int*) 0xC001A400, 0, 320);
		memset((short int*) 0xC001A800, 0, 320);
		memset((short int*) 0xC001AC00, 0, 320);
		memset((short int*) 0xC001B000, 0, 320);
		memset((short int*) 0xC001B400, 0, 320);
		memset((short int*) 0xC001B800, 0, 320);
		memset((short int*) 0xC001BC00, 0, 320);
		memset((short int*) 0xC001C000, 0, 320);
		memset((short int*) 0xC001C400, 0, 320);
		memset((short int*) 0xC001C800, 0, 320);
		memset((short int*) 0xC001CC00, 0, 320);
		memset((short int*) 0xC001D000, 0, 320);
		memset((short int*) 0xC001D400, 0, 320);
		memset((short int*) 0xC001D800, 0, 320);
		memset((short int*) 0xC001DC00, 0, 320);
	}
}