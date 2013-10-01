
#include <stdlib.h>
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
glob global; //buffer work is performed on and wthat is displayed
glob save;	 //buffer of the original image
glob temp;   //temporary work buffer

int copyToBuffer = 0;

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
}


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



//ntsc greyscale
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

//monochrome filter
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


// max value of 3*3 matrix
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

//minimum vluae from 3x3 pixel matrix filter
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

//average mask filter
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



//horizontal sobel filter
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


//vertical sobel filter
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

//custom one filter mask, uses colours from different pixels
//|1|-|-|        |-|-|1|      |-|1|-|
//|1|-|-|        |-|-|1|      |2|-|2|
//|1|-|-|        |-|-|1|      |-|1|-|
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
			//1,0			
			z+=1;
			green += temp[z+w*myIm_Width].g;
			blue  += 2*temp[z+w*myIm_Width].b;
			//2,0			
			z+=1;
			green += temp[z+w*myIm_Width].g;
			//0,1
			w+=1;
			z=x-1;
			blue  += temp[z+w*myIm_Width].b;
			//2,1
			z=x+1;
			blue += temp[z+w*myIm_Width].b;
			//0,2
			z=x-1;
			w=y+1;
			red += temp[z+w*myIm_Width].r;
			//1,2			
			z+=1;
			red += temp[z+w*myIm_Width].r;
			blue  += 2*temp[z+w*myIm_Width].b;
			z+=1;
			//2,2
			red += temp[z+w*myIm_Width].r;

			Im[x+y*myIm_Width].g = red/3;
			Im[x+y*myIm_Width].b = green/3;
			Im[x+y*myIm_Width].r = blue/4;
		
		}
	}
	glutPostRedisplay();
}//custom1

//custom filter mask
// |-1 | 2 | -1|
// |-3 | 3 | -3|
// |-1 | 2 | -1|
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

//custom 3 checkerboard of rgb colours
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
}//custom3 


//custom 4 just weird values
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
}//custom 4

//custom 5 reverse ghost filter
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
}//ciustom 5



//only show red
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



//only show blue
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


// only show green
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


//increase green by 5%
void greenUp(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].g = Im[x+y*myIm_Width].g*1.05;
		}
	}
	glutPostRedisplay();
}//greenUp


//increase red by 5%
void redUp(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].r = Im[x+y*myIm_Width].r*1.05;
		}
	}
	glutPostRedisplay();
}//redUp


//increase blue by 5%
void blueUp(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y,value;
	for (x=0; x < myIm_Width; x++){
		for (y=0; y < myIm_Height; y++){		
			Im[x+y*myIm_Width].b = Im[x+y*myIm_Width].b*1.05;
		}
	}
	glutPostRedisplay();
}//blueUp



//true colour quanitize
void quantize1(pixel* Im, int myIm_Width, int myIm_Height){	
	int x,y,z,w,r,g,b;
	int array[8][3] = {{0,0,0},{255,0,0},{0,255,0},{0,0,255},{255,255,0},{255,0,255},{0,255,255},{255,255,255}};
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			r = Im[x+y*myIm_Width].r;
			g = Im[x+y*myIm_Width].g;
			b = Im[x+y*myIm_Width].b;
			if (r < 128){
				if (g < 128){
					if (b < 128){
						r= array[0][0];
						g= array[0][1];
						b= array[0][2];
					}else{
						r= array[3][0];
						g= array[3][1];
						b= array[3][2];				
					}
				}else{
					if (b < 128){
						r= array[2][0];
						g= array[2][1];
						b= array[2][2];
					}else{
						r= array[6][0];
						g= array[6][1];
						b= array[6][2];	
					}			
				}
			}else{
				if (g < 128){
					if (b < 128){
						r= array[1][0];
						g= array[1][1];
						b= array[1][2];
					}else{
						r= array[5][0];
						g= array[5][1];
						b= array[5][2];				
					}
				}else{
					if (b < 128){
						r= array[4][0];
						g= array[4][1];
						b= array[4][2];
					}else{
						r= array[7][0];
						g= array[7][1];
						b= array[7][2];				
					}
				}
			}
		Im[x+y*myIm_Width].r = r;
		Im[x+y*myIm_Width].b = b;
		Im[x+y*myIm_Width].g = g;
		}
	}
	glutPostRedisplay();
}//quantize1


//random quanatize
void quantize2(pixel* Im, int myIm_Width, int myIm_Height){	
	int x,y,z,w,r,g,b;
	int array[8][3];
	for (int i=0;i<8;i++){
		for(int j=0;j<3;j++){
			array[i][j] = rand()%256;
		}
	}
	for (x=1; x < myIm_Width-1; x++){
		for (y=1; y < myIm_Height-1; y++){
			r = Im[x+y*myIm_Width].r;
			g = Im[x+y*myIm_Width].g;
			b = Im[x+y*myIm_Width].b;
			if (r < 128){
				if (g < 128){
					if (b < 128){
						r= array[0][0];
						g= array[0][1];
						b= array[0][2];
					}else{
						r= array[3][0];
						g= array[3][1];
						b= array[3][2];				
					}
				}else{
					if (b < 128){
						r= array[2][0];
						g= array[2][1];
						b= array[2][2];
					}else{
						r= array[6][0];
						g= array[6][1];
						b= array[6][2];	
					}			
				}
			}else{
				if (g < 128){
					if (b < 128){
						r= array[1][0];
						g= array[1][1];
						b= array[1][2];
					}else{
						r= array[5][0];
						g= array[5][1];
						b= array[5][2];				
					}
				}else{
					if (b < 128){
						r= array[4][0];
						g= array[4][1];
						b= array[4][2];
					}else{
						r= array[7][0];
						g= array[7][1];
						b= array[7][2];				
					}
				}
			}
		Im[x+y*myIm_Width].r = r;
		Im[x+y*myIm_Width].b = b;
		Im[x+y*myIm_Width].g = g;
		}
	}
	glutPostRedisplay();
}//quantize2



//Bonus NPR paint filter
void npr(pixel* Im,pixel* temp, int myIm_Width, int myIm_Height){
	int max = myIm_Width*myIm_Height;
	int x,y,z,w,r,g,b;
	for (int i =0; i < max; i++){
		x = rand() % myIm_Width;
		y = rand() % myIm_Height;
		r = temp[x+y*myIm_Width].r;
		g = temp[x+y*myIm_Width].g;
		b = temp[x+y*myIm_Width].b;
		if (x < 2){x=2;}
		if (x > myIm_Width){ x = myIm_Width;}
		if (y < 2){y=2;}
		if (y > myIm_Height){ x = myIm_Height;}
		for (int j = x-2;j<x+3;j++){
			for(int k = y-2;k<y+3;k++){
				Im[j+k*myIm_Width].r = r;
				Im[j+k*myIm_Width].b = b;
				Im[j+k*myIm_Width].g = g;
			}
		}
	}
	glutPostRedisplay();
}//npr


//reset temp and global buffers to original
void Reset(){
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
	case'r':
    case'R':
		Reset();
		break;
	case't':
	case'T':
		if (copyToBuffer==1){
			copyToBuffer = 0;
		}else{
			copyToBuffer=1;		
		}
		break;
	case'1':
		Reset();
		break;
	case'2':
		grey(global.data,global.w,global.h);
		break;
	case'3':
		ntsc(global.data,global.w,global.h);
		break;
	case'4':
		monochrome(global.data,global.w,global.h);
		break;
	case'5':
		invert(global.data,global.w,global.h);
		break;
	case'6':
		red(global.data,global.w,global.h);
		break;
	case'8':
		blue(global.data,global.w,global.h);
		break;
	case'7':
		green(global.data,global.w,global.h);
		break;
	case'w':
	case'W':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		max(global.data,temp.data,global.w,global.h);
		break;
	case'e':
	case'E':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		min(global.data,temp.data,global.w,global.h);
		break;
	case'y':
	case'Y':
		redUp(global.data,global.w,global.h);
		break;	
	case'u':
	case'U':
		blueUp(global.data,global.w,global.h);
		break;
	case'i':
	case'I':
		greenUp(global.data,global.w,global.h);
		break;
	case'a':
	case'A':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		average(global.data,temp.data,global.w,global.h);
		break;	
	case'd':
	case'D':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		horizontal(global.data,temp.data,global.w,global.h);
		break;	
	case'f':
	case'F':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		vertical(global.data,temp.data,global.w,global.h);
		break;
	case'g':
	case'G':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		vertical(global.data,temp.data,global.w,global.h);
		horizontal(temp.data,global.data,global.w,global.h);
		break;
	case'h':
	case'H':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		custom1(global.data,temp.data,global.w,global.h);
		break;
	case'j':
	case'J':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		custom2(global.data,temp.data,global.w,global.h);
		break;
	case'k':
	case'K':
		printf("\n----------------------\nCustom 1\n----------------------\n");
		printf("  Green        Red        Blue\n");
		printf(" |1|-|-|     |-|-|1|     |-|1|-|\n");
		printf(" |1|-|-|     |-|-|1|     |2|-|2|\n");
		printf(" |1|-|-|     |-|-|1|     |-|1|-|\n");
		printf("\n----------------------\nCustom 2\n----------------------\n");
		printf("|-1| 2|-1|\n");
		printf("| 3|-3| 3|\n");
		printf("|-1| 2|-1|\n");
		break;
	case'b':
	case'B':
		custom3(global.data,global.w,global.h);
		break;
	case'v':
	case'V':
		custom4(global.data,global.w,global.h);
		break;
	case'c':
	case'C':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		custom5(global.data,temp.data,global.w,global.h);;
		break;	
	case'z':
	case'Z':
		quantize1(global.data,global.w,global.h);
		break;
	case'x':
	case'X':
		quantize2(global.data,global.w,global.h);
		break;
	case'M':
	case'm':
		if (copyToBuffer==1){
			memcpy(temp.data, (pixel*)global.data,(global.h)*(global.w)*sizeof(pixel *));
		}
		npr(global.data,temp.data,global.w,global.h);;
		break;	
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
	printf("\n----------------------\nUtilities\n----------------------\n");
	printf("Q:Quit\nR:Reset\nS:Save\nT:Toggle saving image to temp buffer (compounds effects)\n");	
	printf("\n----------------------\n(b) Display\n----------------------\n");
	printf("1:Default\n2:Grey\n3:NTSC\n4:Monochrome\n5:Invert\n6:Red\n7:Green\n8:Blue\n");
	printf("\n----------------------\n(c) Basic Channel Filters\n----------------------\n");
	printf("W:Max\nE:Min\nY:Red + 5%\nU:Blue + 5%\nI:Green + 5%\n");
	printf("\n----------------------\n(d) Mask Filters\n----------------------\n");
	printf("A:average\nD:Sobel Horizontal\nF:Sobel Vertical\nG:Hoziontal & Vertical\nH:Custom 1	\nJ:Custom 2\nK:Print Custom Matricies\n");
	printf("\n----------------------\n(e) Quantize\n----------------------\n");
	printf("Z:True Colour Quanitization\nX:Random Number Quanitization\n");
	printf("\n----------------------\n(f) Custom\n----------------------\n");
	printf("C:Custom - Ghost Mirror\nC:Custom - No Name Here\nB:Custom - RGB Checkers\n");
	printf("\n----------------------\nBONUS\n----------------------\n");
	printf("M:NPR Paint Filter\n");
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);
	
	glutInitWindowSize(global.w,global.h);
	glutCreateWindow("3p98 Image Filter");
	glShadeModel(GL_SMOOTH);
	glutDisplayFunc(display_image);
	glutKeyboardFunc(keyboard);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0,global.w,0,global.h,0,1);
	
	glutMainLoop();

	return 0;
}
