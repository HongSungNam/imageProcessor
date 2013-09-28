/* An example of how to read an image (img.tif) from file using freeimage and then
display that image using openGL's drawPixelCommand. Also allow the image to be saved
to backup.tif with freeimage and a simple thresholding filter to be applied to the image.
Conversion by Lee Rozema.
Added triangle draw routine, fixed memory leak and improved performance by Robert Flack (2008)
*/

#include <stdlib.h>

// Visual Studio 2008 no longer compiles with this include.
// #include <GL/gl.h>

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include "glut.h"
#include "FreeImage.h"

//the pixel structure
typedef struct {
	GLubyte r, g, b;
} pixel;

//the global structure
typedef struct {
	pixel *data;
	int w, h;
} glob;
glob global;
glob save;
glob temp;

//read image
pixel *read_img(char *name, int *width, int *height) {
	FIBITMAP *image;
	int i,j,pnum;
	RGBQUAD aPixel;
	pixel *data;

	if((image = FreeImage_Load(FIF_TIFF, name, 0)) == NULL) {
		return NULL;
	}      
	*width = FreeImage_GetWidth(image);
	*height = FreeImage_GetHeight(image);

	data = (pixel *)malloc((*height)*(*width)*sizeof(pixel *));
	pnum=0;
	for(i = 0 ; i < (*height) ; i++) {
		for(j = 0 ; j < (*width) ; j++) {
			FreeImage_GetPixelColor(image, j, i, &aPixel);
			data[pnum].r = (aPixel.rgbRed);
			data[pnum].g = (aPixel.rgbGreen);
			data[pnum++].b = (aPixel.rgbBlue);
		}
	}
	FreeImage_Unload(image);
	return data;
}//read_img

//write_img
void write_img(char *name, pixel *data, int width, int height) {
	FIBITMAP *image;
	RGBQUAD aPixel;
	int i,j;

	image = FreeImage_Allocate(width, height, 24, 0, 0, 0);
	if(!image) {
		perror("FreeImage_Allocate");
		return;
	}
	for(i = 0 ; i < height ; i++) {
		for(j = 0 ; j < width ; j++) {
			aPixel.rgbRed = data[i*width+j].r;
			aPixel.rgbGreen = data[i*width+j].g;
			aPixel.rgbBlue = data[i*width+j].b;

			FreeImage_SetPixelColor(image, j, i, &aPixel);
		}
	}
	if(!FreeImage_Save(FIF_TIFF, image, name, 0)) {
		perror("FreeImage_Save");
	}
	FreeImage_Unload(image);
}//write_img


/*draw the image - it is already in the format openGL requires for glDrawPixels*/
void display_image(void)
{
	glDrawPixels(global.w, global.h, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);	
	glutPostRedisplay();
	glFlush();
	//glFinish();
}//display_image()

// Read the screen image back to the data buffer after drawing to it
void draw_triangle(void)
{
	glDrawPixels(global.w, global.h, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0,0,0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glColor3f(0,1.0,0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glColor3f(0,0,1.0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glEnd();
	glFlush();
	glReadPixels(0,0,global.w,global.h,GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);
}

/* A simple thresholding filter.
*/
void MyFilter(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;

	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if (Im[x+y*myIm_Width].b > 128)
				Im[x+y*myIm_Width].b = 255;
			else 
				Im[x+y*myIm_Width].b = 0;

			if (Im[x+y*myIm_Width].g > 128)
				Im[x+y*myIm_Width].g = 255;
			else 
				Im[x+y*myIm_Width].g = 0;

			if (Im[x+y*myIm_Width].r > 128)
				Im[x+y*myIm_Width].r = 255;
			else 
				Im[x+y*myIm_Width].r = 0;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void invert(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;

	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){
			Im[x+y*myIm_Width].b = 255 - Im[x+y*myIm_Width].b;
			Im[x+y*myIm_Width].r = 255 - Im[x+y*myIm_Width].r;
			Im[x+y*myIm_Width].g = 255 - Im[x+y*myIm_Width].g;
		}
	}
	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void grey(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){
			value= (Im[x+y*myIm_Width].b + Im[x+y*myIm_Width].r + Im[x+y*myIm_Width].g)/3;
			Im[x+y*myIm_Width].b = value;
			Im[x+y*myIm_Width].r = value;
			Im[x+y*myIm_Width].g = value;
		}
	}
	glutPostRedisplay();
}//grey 




void ntsc(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){
			value= ((0.587*Im[x+y*myIm_Width].b) + (0.299*Im[x+y*myIm_Width].r) + (0.114*Im[x+y*myIm_Width].g))/3;
			Im[x+y*myIm_Width].g = value;
			Im[x+y*myIm_Width].b = value;
			Im[x+y*myIm_Width].r = value;
		}
	}
	glutPostRedisplay();
}//ntsc


void monochrome(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){
			value= ((Im[x+y*myIm_Width].b) + (Im[x+y*myIm_Width].r) + (Im[x+y*myIm_Width].g))/3;
			if (value >= 127){
				value = 255;
			}else{
				value = 0;		
			}			
			Im[x+y*myIm_Width].g = value;
			Im[x+y*myIm_Width].b = value;
			Im[x+y*myIm_Width].r = value;
		}
	}
	glutPostRedisplay();
}//monochrome

void max(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){	
	int x,y,z,w,red,green,blue;
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			red=0;
			green=0;
			blue=0;			
			for(z=x-1;z<x+1;z++){
				for(w=y-1;w<y+1;w++){
					if(green < temp[z+w*myIm_Width].g){
						green = temp[z+w*myIm_Width].g;
					}				
					if(blue < temp[z+w*myIm_Width].b){
						blue = temp[z+w*myIm_Width].b;
					}
					if(red < temp[z+w*myIm_Width].r){
						red = temp[z+w*myIm_Width].r;
					}
									
				}			
			}
			Im[x+y*myIm_Width].g = green;
			Im[x+y*myIm_Width].b = blue;
			Im[x+y*myIm_Width].r = red;
		
		}
	}
	glutPostRedisplay();
}//max

void average(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){	
	int x,y,z,w,red,green,blue;
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			red=0;
			green=0;
			blue=0;			
			for(z=x-1;z<x+1;z++){
				for(w=y-1;w<y+1;w++){
					green += temp[z+w*myIm_Width].g;
					blue += temp[z+w*myIm_Width].b;
					red += temp[z+w*myIm_Width].r;					
				}			
			}
			Im[x+y*myIm_Width].g = green/9;
			Im[x+y*myIm_Width].b = blue/9;
			Im[x+y*myIm_Width].r = red/9;
		
		}
	}
	glutPostRedisplay();
}//average

void horizontal(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){	
	//ntsc(temp,myIm_Width,myIm_Height);
	int x,y,z,w,red,green,blue;
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			red=0;
			green=0;
			blue=0;			
					
			z=x-1;
			w=y-1;
			// 0,0	
			green += temp[z+w*myIm_Width].g;
			blue += temp[z+w*myIm_Width].b;
			red += temp[z+w*myIm_Width].r;					
			//0,1
			z+=1;
			green += 2*temp[z+w*myIm_Width].g;
			blue += 2*temp[z+w*myIm_Width].b;
			red += 2*temp[z+w*myIm_Width].r;	
			//0,2			
			z+=1;
			green += temp[z+w*myIm_Width].g;
			blue += temp[z+w*myIm_Width].b;
			red += temp[z+w*myIm_Width].r;
			//2,2
			w=y+1;
			green -= temp[z+w*myIm_Width].g;
			blue -= temp[z+w*myIm_Width].b;
			red -= temp[z+w*myIm_Width].r;	
			//2,1
			z-=1;
			green -= 2*temp[z+w*myIm_Width].g;
			blue -= 2*temp[z+w*myIm_Width].b;
			red -= 2*temp[z+w*myIm_Width].r;
			//2-0
			z-=1;
			green -= temp[z+w*myIm_Width].g;
			blue -= temp[z+w*myIm_Width].b;
			red -= temp[z+w*myIm_Width].r;	
			//stuff
			Im[x+y*myIm_Width].g = green/9;
			Im[x+y*myIm_Width].b = blue/9;
			Im[x+y*myIm_Width].r = red/9;
		
		}
	}
	glutPostRedisplay();
}//horizontal



void vertical(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){	
	//ntsc(temp,myIm_Width,myIm_Height);
	int x,y,z,w,red,green,blue;
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			red=0;
			green=0;
			blue=0;				
			z=x-1;
			w=y-1;
			// 0,0	
			green += temp[z+w*myIm_Width].g;
			blue += temp[z+w*myIm_Width].b;
			red += temp[z+w*myIm_Width].r;					
			//0,1
			w+=1;
			green += 2*temp[z+w*myIm_Width].g;
			blue += 2*temp[z+w*myIm_Width].b;
			red += 2*temp[z+w*myIm_Width].r;	
			//0,2			
			w+=1;
			green += temp[z+w*myIm_Width].g;
			blue += temp[z+w*myIm_Width].b;
			red += temp[z+w*myIm_Width].r;
			//2,2
			z=x+1;
			green -= temp[z+w*myIm_Width].g;
			blue -= temp[z+w*myIm_Width].b;
			red -= temp[z+w*myIm_Width].r;	
			//2,1
			w-=1;
			green -= 2*temp[z+w*myIm_Width].g;
			blue -= 2*temp[z+w*myIm_Width].b;
			red -= 2*temp[z+w*myIm_Width].r;
			//2-0
			w-=1;
			green -= temp[z+w*myIm_Width].g;
			blue -= temp[z+w*myIm_Width].b;
			red -= temp[z+w*myIm_Width].r;	
			//stuff
			Im[x+y*myIm_Width].g = green/9;
			Im[x+y*myIm_Width].b = blue/9;
			Im[x+y*myIm_Width].r = red/9;
		
		}
	}
	glutPostRedisplay();
}//vertical

void custom1(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){	
	//ntsc(temp,myIm_Width,myIm_Height);
	int x,y,z,w,red,green,blue;
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			red=0;
			green=0;
			blue=0;				
			z=x-1;
			w=y-1;
			// 0,0	
			green += temp[z+w*myIm_Width].g;
			//0,1			
			z+=1;
			green += temp[z+w*myIm_Width].g;
			blue  += 2*temp[z+w*myIm_Width].b;
			//0,2			
			z+=1;
			green += temp[z+w*myIm_Width].g;
			//1,0
			w+=1;
			z=x-1;
			blue  += temp[z+w*myIm_Width].b;
			//1,2
			z=x+1;
			blue += temp[z+w*myIm_Width].b;
			//2,0
			z=x-1;
			w=y+1;
			red += temp[z+w*myIm_Width].r;
			z+=1;
			red += temp[z+w*myIm_Width].r;
			blue  += 2*temp[z+w*myIm_Width].b;
			z+=1;
			red += temp[z+w*myIm_Width].r;

			Im[x+y*myIm_Width].g = red/3;
			Im[x+y*myIm_Width].b = green/3;
			Im[x+y*myIm_Width].r = blue/4;
		
		}
	}
	glutPostRedisplay();
}//custom1

void custom2(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){	
	//ntsc(temp,myIm_Width,myIm_Height);
	int x,y,z,w,red,green,blue,factor;
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			red=0;
			green=0;
			blue=0;				
			z=x-1;
			w=y-1;
			// 0,0	
			factor = -1;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;
			//0,1			
			z+=1;
			factor = 3;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;
			//0,2			
			z+=1;
			factor = -1;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;
			//1,0
			w+=1;
			z=x-1;
			factor = 2;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;
			//1,1
			z=x+1;
			factor = -3;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;
			//1,2
			z++;
			factor = 2;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;
			//2,0
			z=x-1;
			w=y+1;
			factor = -1;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;
			//0,1			
			z+=1;
			factor = 3;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;
			//0,2			
			z+=1;
			factor = -1;
			green +=  factor * temp[z+w*myIm_Width].g;
			red+= factor * temp[z+w*myIm_Width].r;
			blue += factor * temp[z+w*myIm_Width].b;

			Im[x+y*myIm_Width].g = red/3;
			Im[x+y*myIm_Width].b = green/3;
			Im[x+y*myIm_Width].r = blue/4;
		
		}
	}
	glutPostRedisplay();
}//custom2

void custom3(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){
			if (y % 60 < 20){
				if (x % 60 <20){
					Im[x+y*myIm_Width].b =255;
					Im[x+y*myIm_Width].r =0;
				}else if(x % 60 < 40 && x % 60 >19 ){
					Im[x+y*myIm_Width].g =0;
					Im[x+y*myIm_Width].r =255;
				}else if(x % 60 > 39){
					Im[x+y*myIm_Width].b =255;
					Im[x+y*myIm_Width].g =0;
				}
			}else if(y % 60 < 40 && y%60>19){
				if (x % 60 <20){
					Im[x+y*myIm_Width].g =0;
					Im[x+y*myIm_Width].r =255;
				}else if(x % 60 < 40 && x % 60 >19 ){
					Im[x+y*myIm_Width].g =255;
					Im[x+y*myIm_Width].b =0;
				}else if(x % 60 > 39){
					Im[x+y*myIm_Width].b =255;
					Im[x+y*myIm_Width].r =0;
				}
			}else if (y % 60 > 39){
				if (x % 60 <20){
					Im[x+y*myIm_Width].b =255;
					Im[x+y*myIm_Width].g =0;
				}else if(x % 60 < 40 && x % 60 >19 ){
					Im[x+y*myIm_Width].b =0;
					Im[x+y*myIm_Width].r =255;
				}else if(x % 60 > 39){
					Im[x+y*myIm_Width].g =255;
					Im[x+y*myIm_Width].r =0;
				}
			}	

		}
	}
	glutPostRedisplay();
}//monochrome

void custom4(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value,red,blue,green;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){
			red = Im[x+y*myIm_Width].r;
			blue = Im[x+y*myIm_Width].b;
			green = Im[x+y*myIm_Width].g;
			if (red > blue && red > green){
				if (green > blue){
				//	red = floor(red*(blue*0.85));
					//blue = floor(blue*(green*0.639));
					//green = floor(255 - green*0.7);				
				}else{
				//	red = floor(blue*0.5 + green*0.7);
					green = 255;
					blue = 0;				
				}
			}else if(blue > red && blue > green){
				if ( red > green){
					blue = 255 - green;
					//red = floor((red*red) / 4);
					//green =floor(0.8*(red + green)); 
				}else{
					if (x > 0 && y > 0){
						red = x % y;
						if (red*2> 255){
							green = 255 - red - blue;
						}else{
							green = 255 - red*2;
						}
					}else{
						red = 0;			
					}
					//green =floor(blue*0.25*red);
					//blue =floor( blue*1.1);
				}			
			}else{
				if (red > blue){
					if (x > 0 && y > 0){					
						red = y % x;
					}else{red = 255;}
					//blue = floor(red*1.4);
					green = 255-y-x;
				}else{
					red = 255-red;;
					//green = (((blue-red)*blue/red)/red)*blue;
					if (blue > 128){
						blue = 255;
					}else{
						blue = 0;				
					}			
				}
			}
			Im[x+y*myIm_Width].g = green;
			Im[x+y*myIm_Width].b = blue;
			Im[x+y*myIm_Width].r = red;
		}
	}
	glutPostRedisplay();
}//monochrome

void custom5(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){	
	int x,y,z,w,red,green,blue,factor;
	average(Im,temp,global.w,global.h);	
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			Im[+y*myIm_Width].g = temp[(myIm_Width-x)+y*myIm_Width].r;	
			Im[x+y*myIm_Width].r = temp[(myIm_Width-x)+y*myIm_Width].g;
		}
	}
	glutPostRedisplay();
}

void min(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){	
	int x,y,z,w,red,green,blue;
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			red=255;
			green=255;
			blue=255;			
			for(z=x-1;z<x+1;z++){
				for(w=y-1;w<y+1;w++){
					if(green > temp[z+w*myIm_Width].g){
						green = temp[z+w*myIm_Width].g;
					}				
					if(blue > temp[z+w*myIm_Width].b){
						blue = temp[z+w*myIm_Width].b;
					}
					if(red > temp[z+w*myIm_Width].r){
						red = temp[z+w*myIm_Width].r;
					}
									
				}			
			}
			Im[x+y*myIm_Width].g = green;
			Im[x+y*myIm_Width].b = blue;
			Im[x+y*myIm_Width].r = red;
		
		}
	}
	glutPostRedisplay();
}//min

void red(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].g = 255;
			Im[x+y*myIm_Width].b = 255;

		}
	}
	glutPostRedisplay();
}//red


void blue(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].g = 255;
			Im[x+y*myIm_Width].r = 255;

		}
	}
	glutPostRedisplay();
}//blue

void green(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].r = 255;
			Im[x+y*myIm_Width].b = 255;

		}
	}
	glutPostRedisplay();
}//green

void greenUp(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].g = Im[x+y*myIm_Width].g*1.05;
		}
	}
	glutPostRedisplay();
}//greenUp

void redUp(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].r = Im[x+y*myIm_Width].r*1.05;
		}
	}
	glutPostRedisplay();
}//redUp

void blueUp(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].b = Im[x+y*myIm_Width].b*1.05;
		}
	}
	glutPostRedisplay();
}//blueUp



void Reset(){//pixel* global,pixel* save){
	memcpy(global.data, (pixel*)save.data,(global.h)*(global.w)*sizeof(pixel *));
	memcpy(temp.data, (pixel*)save.data,(global.h)*(global.w)*sizeof(pixel *));    
	glutPostRedisplay();
}

/*glut keyboard function*/
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 0x1B:
	case'q':
	case 'Q':
		exit(0);
		break;
	case's':
	case'S':
		printf("SAVING IMAGE: backup.tif\n");
		write_img("backup.tif", global.data, global.w, global.h);
		break;
	case 't':
	case 'T':
		draw_triangle();
		break;
	case'f':
	case'F':
		MyFilter(global.data,global.w,global.h);
		break;
	case'r':
    case'R':
		Reset();//global.data,global.w,global.h);
		break;
	case'g':
	case'G':
		grey(global.data,global.w,global.h);
		break;
	case'n':
	case'N':
		ntsc(global.data,global.w,global.h);
		break;
	case'm':
	case'M':
		monochrome(global.data,global.w,global.h);
		break;
	case'i':
	case'I':
		invert(global.data,global.w,global.h);
		break;
	case'1':
		red(global.data,global.w,global.h);
		break;
	case'2':
		blue(global.data,global.w,global.h);
		break;
	case'3':
		green(global.data,global.w,global.h);
		break;
	case'4':
		max(global.data,temp.data,global.w,global.h);
		break;
	case'5':
		min(global.data,temp.data,global.w,global.h);
		break;
	case'6':
		redUp(global.data,global.w,global.h);
		break;	
	case'7':
		blueUp(global.data,global.w,global.h);
		break;
	case'8':
		greenUp(global.data,global.w,global.h);
		break;
	case'9':
		average(global.data,temp.data,global.w,global.h);
		break;	
	case'0':
		horizontal(global.data,temp.data,global.w,global.h);
		break;	
	case'v':
		vertical(global.data,temp.data,global.w,global.h);
		break;
	case'd':
		vertical(global.data,temp.data,global.w,global.h);
		horizontal(temp.data,global.data,global.w,global.h);
		break;
	case'x':
		custom1(global.data,temp.data,global.w,global.h);
		break;
	case'b':
		custom2(global.data,temp.data,global.w,global.h);
		break;
	case'k':
		custom3(global.data,global.w,global.h);
		break;
	case'j':
		custom4(global.data,global.w,global.h);
		break;
	case'u':
		custom5(global.data,temp.data,global.w,global.h);;
	

	}
}//keyboard


int main(int argc, char** argv)
{
	save.data = read_img("img.tif", &global.w, &global.h);
    global.data = read_img("img.tif", &save.w, &save.h);
	temp.data = read_img("img.tif", &temp.w, &temp.h);
	if (global.data==NULL)
	{
		printf("Error loading image file img.tif\n");
		return 1;
	}
	printf("Q:quit\nF:filter\nG:grey\nT:triangle\nS:save\nR:reset\n");
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);
	
	glutInitWindowSize(global.w,global.h);
	glutCreateWindow("SIMPLE DISPLAY");
	glShadeModel(GL_SMOOTH);
	glutDisplayFunc(display_image);
	glutKeyboardFunc(keyboard);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0,global.w,0,global.h,0,1);
	
	glutMainLoop();

	return 0;
}
