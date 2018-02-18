
/*
	laser.C -- laser graffiti program
	
	Copyright (C) 2007 Jeffrey Sharkey, jsharkey.org
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


// here is the latest working command line
// it runs smoothly on my core2 box with nvidia card
// make && dvgrab --every 30 --format raw - | ffmpeg -f dv -i - -f rawvideo -vcodec rawvideo -pix_fmt bgr24 -r 15 - | ./plot


// this is absolutely aweful coding below, as most of the work was
// done during a 36-hour marathon session with little sleep

// when running, use the F-keys to control paint type and
// clear screen, or escape to quit




#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glpng.h>

#include <math.h>
#include "SDL.h"



#include <termios.h>
#include <fcntl.h>

//#include "TGALoader.h"
#include "Texture.h"

bool LoadTGA(Texture * ,const char * );



#include <cstdlib> 


#include "gd.h"

#include <stdio.h>
#include <iostream>

using namespace std;

#define WIDTH 720
#define HEIGHT 480

//#define WIDTH 540
//#define HEIGHT 360


#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define SCREEN_BPP     16

#define TRUE  1
#define FALSE 0


#define RED 1
#define GREEN 3
#define BLUE 2


//GLuint  texture;
Texture *texture;

Texture lib5[10], lib6[10], lib7[10], lib8[10];
int max5 = 0, max6 = 0, max7 = 0, max8 = 0;
int max0 = 0;

Texture halo;

pngInfo info;


float size = 0.1;


// keep track of all points actually drawn by the user
// separate each line segment by values -1024,-1024

int fcount;
int* fx;
int* fy;


int* gestx;
int* gesty;
int skip=0;



int currentbrush = -1;

void changebrush(int i) {
	currentbrush = i;
	switch(i) {
		case 5: texture = lib5; max0 = max5; break;
		case 6: texture = lib6; max0 = max6; break;
		case 7: texture = lib7; max0 = max7; break;
		case 8: texture = lib8; max0 = max8; break;
	}
}





bool ispeak(int r, int g, int b, int color) {
	int min = 16;  // 16 outside
	float thresh = 0.5f;  // 0.6 outside
	int t = -1;
	switch(color) {
		case RED:
			if(r < min) return false;
			t = thresh*r;
			return (g < t && b < t);
			break;
		case GREEN:
			if(g < min) return false;
			t = thresh*g;
			return (r < t && b < t);
			break;
		case BLUE:
			if(b < min) return false;
			t = thresh*b;
			return (r < t && g < t);
			break;
	}
	
}




// handle calibration settings
bool iscalibrated = false;
int xa,ya,ca;
int xb,yb,cb;
int xc,yc,cc;
int xd,yd,cd;


void trycalibration() {
	// read in an image and try calibrating if enough information
	xa=ya=ca=0;
	xb=yb=cb=0;
	xc=yc=cc=0;
	xd=yd=cd=0;
	
	//gdImagePtr im = gdImageCreateTrueColor(WIDTH, HEIGHT);
	
	float small = 128, large = -128;
	
	for(int y = 0; y < HEIGHT; y++) {
		for(int x = 0; x < WIDTH; x++) {
			
			// read in three bytes for each pixel color
			unsigned char r, g, b;
			b = (unsigned char) getchar();
			g = (unsigned char) getchar();
			r = (unsigned char) getchar();
			
/*
			float rr = r, gg = g, bb = b;
			float comp = gg/(rr+gg+bb);
			
			if(comp > large) large = comp;
			if(comp < small) small = comp;
			
			//gg = ((gg*3*255)/(rr+gg+bb));
			//g = (unsigned char)gg;
			//r=b=0;
			//g = ((g*3*255)/(r+g+b));
			
			if(g > 64 && comp > 0.5) {
			//cout << "has r="<<(int)r<<",g="<<(int)g<<",b="<<(int)b<<endl;
			g = comp*255;r=b=0;
			int color = gdTrueColor(r, g, b);
			gdImageSetPixel(im, x, y, color);
			}
*/			
			// based on the current quadrant, find the calibration image
			if(x < WIDTH/2 && y < HEIGHT/2) {
				// topleft blue
				if(ispeak(r,g,b,GREEN)) {
					xa+=x;ya+=y;ca++;
					cout << "found greenpixel at x=" << x << ",y=" << y << endl;
				}
			} else if(x > WIDTH/2 && y < HEIGHT/2) {
				// topright green
				if(ispeak(r,g,b,GREEN)) {
					xb+=x;yb+=y;cb++;
					//cout << "found green pixel at x=" << x << ",y=" << y << endl;
				}
			} else if(x < WIDTH/2 && y > HEIGHT/2) {
				// bottomleft green
				if(ispeak(r,g,b,GREEN)) {
					xc+=x;yc+=y;cc++;
				}
			} else if(x > WIDTH/2 && y > HEIGHT/2) {
				// bottomright blue
				if(ispeak(r,g,b,GREEN)) {
					xd+=x;yd+=y;cd++;
				}
			}
			
		}
	}
	
	/** /
	FILE *pngout;
	pngout = fopen("test.png", "wb");
	gdImagePng(im, pngout);
	fclose(pngout);
/**/

	cout << "over whole frame small=" << small << " and large=" << large << endl;

	int mincount = 2;
	// check to make sure we have a good calibration
	if(ca < mincount || cb < mincount || cc < mincount || cd < mincount)
		return;
		
	xa /= ca; ya /= ca;
	xb /= cb; yb /= cb;
	xc /= cc; yc /= cc;
	xd /= cd; yd /= cd;
	
	/**/
	cout << "xa=" << xa << ",ya=" << ya << endl;
	cout << "xb=" << xb << ",yb=" << yb<< endl;
	cout << "xc=" << xc << ",yc=" << yc<< endl;
	cout << "xd=" << xd << ",yd=" << yd<< endl;
	/**/
	
	iscalibrated = true;

}




void parseCentroid(int* x, int* y) {
	
	// read an image from stdin and try finding the red centroid
	//gdImagePtr im = gdImageCreateTrueColor(WIDTH, HEIGHT);

	int ax = 0, ay = 0;
	int cx = 0, cy = 0;
	
	// walk through setting pixels
	for(int y = 0; y < HEIGHT; y++) {
		for(int x = 0; x < WIDTH; x++) {
		
			// read in three bytes for each pixel color
			unsigned char r, g, b;
			b = (unsigned char) getchar();
			g = (unsigned char) getchar();
			r = (unsigned char) getchar();
			
			// try masking out all colors that are relatively red
			if(r < 48) continue;
			
			// red channel needs to be 20% more than both other color channels
			int thresh = 0.4f*r;
			if(g < thresh && b < thresh) {
				//int color = gdTrueColor(r, g, b);
				//gdImageSetPixel(im, x, y, color);
				
				ax += x; ay += y;
				cx++; cy++;
				
			}
		
		}
	}
	
	/** /
	FILE *pngout;
	pngout = fopen("test.png", "wb");
	gdImagePng(im, pngout);
	fclose(pngout);
/**/
	
	if(cx < 5) {
		*x = -1024;
		*y = -1024;
	} else {
		ax /= cx; ay /= cy;
		*x = ax; *y = ay;
	}
	
}








SDL_Surface *surface;




void Quit( int returnCode ) {
    SDL_Quit( );
    exit( returnCode );
}

/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height )
{
    /* Height / width ration */
    GLfloat ratio;
 
    /* Protect against a divide by zero */
   if ( height == 0 )
	height = 1;

    ratio = ( GLfloat )width / ( GLfloat )height;

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    /* Set our perspective */
    gluPerspective( 45.0f, ratio, 0.1f, 100.0f );

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );

    return( TRUE );
}

/* function to handle key press events */
void handleKeyPress( SDL_keysym *keysym )
{
    switch ( keysym->sym )
	{
	case SDLK_ESCAPE:
	    /* ESC key was pressed */
	    Quit( 0 );
	    break;
	case SDLK_F1: fcount = 0; break;
	
	case SDLK_F3: max0 = -1; break;
	case SDLK_F4: max0 = -2; break;

	case SDLK_F5: changebrush(5); break;
	case SDLK_F6: changebrush(6); break;
	case SDLK_F7: changebrush(7); break;
	case SDLK_F8: changebrush(8); break;

	case SDLK_UP: size += 0.05; size = (float)min(max((double)size,0.05),0.5); break;
	case SDLK_DOWN: size -= 0.05; size = (float)min(max((double)size,0.05),0.5); break;


	default:
	    break;
	}

    return;
}

/* general OpenGL initialization function */
int initGL( GLvoid )
{

    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	//glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* Enables Depth Testing */
    glEnable( GL_DEPTH_TEST );

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Really Nice Perspective Calculations */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    //our_font.init("Test.ttf", 16);

SDL_ShowCursor (SDL_DISABLE);

    return( TRUE );
}



/** /
// old conversion of points
// dump entire string of paths so far
float x = fx[i] / 720.0f;
float y = fy[i] / 480.0f;
x*=4;x-=2;y*=2;y=1-y;
/**/


void homo(float inx, float iny, float* outx, float* outy) {
	
	// remember we have a -1.0 to 1.0 area to plot into
	// find the ratio along each of the calibration edges
	float fyright = (iny-yb) / (yd-yb),
		fyleft = (iny-ya) / (yc-ya),
		fxtop = (inx-xa) / (xb-xa),
		fxbot = (inx-xc) / (xd-xc);
		
	float fright = ((xd-xb)*fyright)+xb,
		fleft = ((xc-xa)*fyleft)+xa;
	float ftop = ((yb-ya)*fxtop)+ya,
		fbot = ((yd-yc)*fxbot)+yc;
	
	float x = (inx-fleft) / (fright-fleft);
	float y = 1-((iny-ftop) / (fbot-ftop));
	
	x*=2;x-=1;
	y*=1.6;y-=0.7;
	
	*outx = x;
	*outy = y;
}

#define PI 3.14159

double toradians(double d) {
	return d * PI / 180;
}

double bearing(float x1, float y1, float x2, float y2) {
	x1 = toradians(x1);
	y1 = toradians(y1);
	x2 = toradians(x2);
	y2 = toradians(y2);
	
	return atan2(sin(y2 - y1) * cos(x2),
		cos(x1) * sin(x2) -
		sin(x1) * cos(x2) * cos(y2 - x1));
}


void project(float x1, float y1, double bearing, double distance, float* x, float* y) {
	x1 = toradians(x1);
	y1 = toradians(y1);
	double ratio = distance/1;

	double x2 = asin(sin(x1) * cos(ratio) +
		cos(x1) * sin(ratio) * cos(bearing));
	double y2 = x1 + atan2(sin(bearing) * sin(ratio) * cos(x1),
		cos(ratio) - sin(x1) * sin(x2));
	
	*x = x2;
	*y = y2;
}


int lastcount = -10;



/* Here goes our drawing code */
int drawGLScene(GLvoid) {

	if(fcount == lastcount && fcount != 0) {
		return(TRUE);
	}



	if(!iscalibrated) {
		// draw calibration image in corners of viewport
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0.0f,-0.2f,-2.5f);

		float size = 0.05;
		
		// top-left corner
		glColor4ub(0x00,0xff,0x00,0xff);
		glBegin(GL_QUADS);
		glVertex3f(-1.0f, 0.9f, 0.0f);
		glVertex3f(-1.0f+size, 0.9f, 0.0f);
		glVertex3f(-1.0f+size, 0.9f+size, 0.0f);
		glVertex3f(-1.0f, 0.9f+size, 0.0f);
		glEnd();
		
		// top-right corner
		glColor4ub(0x00,0xff,0x00,0xff);
		glBegin(GL_QUADS);
		glVertex3f(1.0f, 0.9f, 0.0f);
		glVertex3f(1.0f-size, 0.9f, 0.0f);
		glVertex3f(1.0f-size, 0.9f+size, 0.0f);
		glVertex3f(1.0f, 0.9f+size, 0.0f);
		glEnd();
		
		// bottom-left corner
		glColor4ub(0x00,0xff,0x00,0xff);
		glBegin(GL_QUADS);
		glVertex3f(-1.0f, -0.7f, 0.0f);
		glVertex3f(-1.0f+size, -0.7f, 0.0f);
		glVertex3f(-1.0f+size, -0.7f+size, 0.0f);
		glVertex3f(-1.0f, -0.7f+size, 0.0f);
		glEnd();

		// bottom-right corner
		glColor4ub(0x00,0xff,0x00,0xff);
		glBegin(GL_QUADS);
		glVertex3f(1.0f, -0.7f, 0.0f);
		glVertex3f(1.0f-size, -0.7f, 0.0f);
		glVertex3f(1.0f-size, -0.7f+size, 0.0f);
		glVertex3f(1.0f, -0.7f+size, 0.0f);
		glEnd();

		

	
		/*
		glVertex3f( 0.0f, 1.0f, 0.0f);				// Top
		glVertex3f(-1.0f,-1.0f, 0.0f);				// Bottom Left
		glVertex3f( 1.0f,-1.0f, 0.0f);				// Bottom Right
		*/

	} else {
		if(fcount == 0) {
			//cout << "trying to clear" << endl;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//glLoadIdentity();
			//glTranslatef(0.0f,0.0f,-2.5f);
		}

		// draw the actual line segments so far onto screen
		
		
		float q = 1.;
			
		//glBegin(GL_LINE_STRIP);
		//glEnable(GL_LINE_SMOOTH);
		for(int i = lastcount; i < fcount; i++) {
			if(fx[i-1] == -1024 || fx[i] == -1024) {
				//glEnd();
				//glBegin(GL_LINE_STRIP);
				continue;
			}
			
			float x1,y1,x2,y2;
			
			homo((float)fx[i-1], (float)fy[i-1], &x1, &y1);
			homo((float)fx[i], (float)fy[i], &x2, &y2);
			
			if(y1 < -1 || y2 < -1 || y1 > 2 || y2 > 2)
				continue;
			
			//glBegin(GL_POLYGON);
			
			/*
			float size = 5;
			
			double bear = bearing(x1, y1, x2, y2);
			
			float x6,y6,x7,y7,x8,y8,x9,y9;
			project(x1,y1,bear+(PI/2),size,&x6,&y6);
			project(x1,y1,bear-(PI/2),size,&x7,&y7);
			project(x2,y2,bear+(PI/2),size,&x8,&y8);
			project(x2,y2,bear-(PI/2),size,&x9,&y9);
			
			glVertex3f(x6, y6, 0.0f);
			glVertex3f(x7, y7, 0.0f);
			glVertex3f(x8, y8, 0.0f);
			glVertex3f(x9, y9, 0.0f);
			*/
			
			/** /
			glVertex3f(x1+size, y1+size, 0.0f);
			glVertex3f(x1-size, y1+size, 0.0f);
			glVertex3f(x1-size, y1-size, 0.0f);
			
			glVertex3f(x2-size, y2-size, 0.0f);
			glVertex3f(x2+size, y2-size, 0.0f);
			glVertex3f(x2+size, y2+size, 0.0f);
			/**/
			
			//glEnd();


if(max0 == -1 || max0 == -2) {

glDisable(GL_TEXTURE_2D);
glDisable(GL_BLEND);
			glColor4ub(0x00,0x88,0xff,0xff);
			float length = sqrt(pow(x2-x1,2)+pow(y2-y1,2));
if(max0 == -1) {
	glLineWidth(length*80);
} else {
	glLineWidth(100.0f);
}
			glBegin(GL_LINES);
			glVertex3f(x1, y1, 0.0f);
			glVertex3f(x2, y2, 0.0f);
			glEnd();

} else {

// fill in the area as we run along the line
glEnable(GL_TEXTURE_2D);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//float size = 0.1;
float step = 0.005;

//float temp;
//if(x1 > x2) { temp=x1; x1=x2; x2=temp; }
//if(y1 > y2) { temp=y1; y1=y2; y2=temp; }

float length = sqrt(pow(x2-x1,2)+pow(y2-y1,2));
float steps = length / step;

float dx = (x2-x1) / steps;
float dy = (y2-y1) / steps;
float px = x1;
float py = y1;


int i = 0;

for(float d = 0; d < length; d += step) {
	px += dx;
	py += dy;

	// try drawing halo first
/*
	glColor4ub(0xff,0xff,0xff,0x8);
	glBindTexture(GL_TEXTURE_2D, halo.texID);

	glBegin(GL_QUADS);
		glTexCoord2f(1, 1); glVertex3f(px-size, py-size, 0);
		glTexCoord2f(1, 0); glVertex3f(px+size, py-size, 0);
		glTexCoord2f(0, 0); glVertex3f(px+size, py+size, 0);
		glTexCoord2f(0, 1); glVertex3f(px-size, py+size, 0);
	glEnd();
*/

	// draw actual brush
	i = (int) (((double)rand() * (double)max0) / (double)RAND_MAX);
	glColor4ub(0xff,0xff,0xff,0x11);
	glBindTexture(GL_TEXTURE_2D, texture[i].texID);

	glBegin(GL_QUADS);
		glTexCoord2f(1, 1); glVertex3f(px-size, py-size, 0);
		glTexCoord2f(1, 0); glVertex3f(px+size, py-size, 0);
		glTexCoord2f(0, 0); glVertex3f(px+size, py+size, 0);
		glTexCoord2f(0, 1); glVertex3f(px-size, py+size, 0);
	glEnd();
}


}




			/** /
			// line width based on distance
			// *************************************************************** WORKING LINE CODE
			float length = sqrt(pow(x2-x1,2)+pow(y2-y1,2));
			
			glLineWidth(length*80);
			//glLineWidth(100.0f);
			glBegin(GL_LINES);
			glVertex3f(x1, y1, 0.0f);
			glVertex3f(x2, y2, 0.0f);
			glEnd();
			// *************************************************************** WORKING LINE CODE
			/**/
		}

	}

	lastcount = fcount;

cout << "done rendering with fcount=" << fcount << endl;

	SDL_GL_SwapBuffers( );
	return( TRUE );
}






float xabs(float x) {
	if(x < 0) return -x;
	return x;
}





bool trygesture() {
	// look through gestx,gesty for gesture

	// find the lowerleft and upperright opengl coords
	float x1,y1,x2,y2;
	homo(360., 1., &x1, &y1);
	homo(360., 480., &x2, &y2);

	float tolerance = 0.4;
	// try looking for clear motion (from lowerleft to upperright)
	// needs to hit both corners
	bool hit1 = false,
		hit2 = false;
	for(int i = 0; i < skip; i++) {
		float x, y;
		homo((float)gestx[i], (float)gesty[i], &x, &y);
		//cout << "\tx1=" << xabs(x-x1); cout << "\ty1=" << xabs(y-y1); cout << "\t\tx2=" << xabs(x-x2); cout << "\ty2=" << xabs(y-y2) << endl;
		if((xabs(x-x1) < tolerance) && (xabs(y-y1) < tolerance)) hit1=true;
		if((xabs(x-x2) < tolerance) && (xabs(y-y2) < tolerance)) hit2=true;
		
	}
	
	cout << "HIT1=" << hit1 << "\tHIT2=" << hit2 << endl;





	// NOW try alternating through different paint brushes
	homo(0., 0., &x1, &y1);
	homo(720., 480., &x2, &y2);

	tolerance = 0.5;
	hit1 = false; hit2 = false;
	for(int i = 0; i < skip; i++) {
		float x, y;
		homo((float)gestx[i], (float)gesty[i], &x, &y);
		if((xabs(x-x1) < tolerance) && (xabs(y-y1) < tolerance)) hit1=true;
		if((xabs(x-x2) < tolerance) && (xabs(y-y2) < tolerance)) hit2=true;
	}

	cout << "HIT1=" << hit1 << "\tHIT2=" << hit2 << endl;
	
	if(hit1 && hit2) {
		cout << "ABOUT TO CLEAR SCREEN" << endl;
		fcount = 0;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return TRUE;
	}

/*
	if(hit1 && hit2) {
		cout << "ABOUT TO CHANGE BRUSH" << endl;
		currentbrush++;
		if(currentbrush == 9) currentbrush = 5;
		changebrush(currentbrush);

		return TRUE;
	}
*/


	return FALSE;
}







int main(void) {

	
	fcount = 0;
	fx = new int[5000];
	fy = new int[5000];

	gestx = new int[10];
	gesty = new int[10];

    /* Flags to pass to SDL_SetVideoMode */
    int videoFlags;
    /* main loop variable */
    int done = FALSE;
    /* used to collect events */
    SDL_Event event;
    /* this holds some info about our display */
    const SDL_VideoInfo *videoInfo;
    /* whether or not the window is active */
    int isActive = TRUE;

    /* initialize SDL */
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
	    fprintf( stderr, "Video initialization failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

    /* Fetch the video info */
    videoInfo = SDL_GetVideoInfo( );

    if ( !videoInfo )
	{
	    fprintf( stderr, "Video query failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

    /* the flags to pass to SDL_SetVideoMode */
    videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
    //videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
    videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */

    /* This checks to see if surfaces can be stored in memory */
    if ( videoInfo->hw_available )
	videoFlags |= SDL_HWSURFACE;
    else
	videoFlags |= SDL_SWSURFACE;

    /* This checks if hardware blits can be done */
    if ( videoInfo->blit_hw )
	videoFlags |= SDL_HWACCEL;

    /* Sets up OpenGL double buffering */
    //SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    /* get a SDL surface */
    surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
				videoFlags );

    /* Verify there is a surface */
    if ( !surface )
	{
	    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
	    Quit( 1 );
	}

    /* initialize OpenGL */
    initGL( );

    /* resize the initial window */
	resizeWindow( SCREEN_WIDTH, SCREEN_HEIGHT );
	//SDL_WM_ToggleFullScreen( surface );

	//lib5[0].texID = pngBind("graffiti1.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	//max5 = 1;
	
	//lib6[0].texID = pngBind("graffiti3.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	//lib6[1].texID = pngBind("graffiti4.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	//max6 = 2;

	halo.texID = pngBind("halo1.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);

	lib5[0].texID = pngBind("0graffiti4.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	lib5[1].texID = pngBind("0graffiti5.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	lib5[2].texID = pngBind("0graffiti6.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	max5 = 3;

	lib6[0].texID = pngBind("graffiti7.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	lib6[1].texID = pngBind("graffiti8.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	lib6[2].texID = pngBind("graffiti9.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	max6 = 3;

	lib7[0].texID = pngBind("graffiti10.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	lib7[1].texID = pngBind("graffiti11.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	lib7[2].texID = pngBind("graffiti12.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	max7 = 3;

	lib8[0].texID = pngBind("graffiti13.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	lib8[1].texID = pngBind("graffiti14.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	lib8[2].texID = pngBind("graffiti15.png", PNG_NOMIPMAP, PNG_ALPHA, &info, GL_CLAMP, GL_NEAREST, GL_NEAREST);
	max8 = 3;
	
	changebrush(5);

	//texture = lib5;
	//max0 = max5;
	//currentbrush = 5;

/*
	LoadTGA(&texture, "graffiti1.tga");
	glGenTextures(1, &(texture.texID));
	glBindTexture(GL_TEXTURE_2D, texture.texID);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.imageData);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
*/


//fflush(stdout); exit(0);


int x,y;
int n;

// make && dvgrab --format raw - | ffmpeg -r 1 -f dv -i - -f rawvideo -vcodec rawvideo -pix_fmt bgr24 - | ./plot

	/* wait for events */ 
	while (!done) {
	
	//tcflush(fileno(stdin),TCIFLUSH);
	
		if(!iscalibrated) {
			// try calibration if not already done
			trycalibration();
			//if(n++ < 1)
			//	iscalibrated = false;
			cout << "iscalibrated=" << iscalibrated  << endl;
			if(iscalibrated)
				fcount = 0;
		} else {
			// otherwise parse the incoming image
			parseCentroid(&x, &y);
			if(x != -1024) {

				if(skip++ > 10) {
					// before drawing, try detecting a gesture
//cout << "has skip=" << skip <<endl;
					if(skip == 11 && trygesture() == TRUE) {
						skip = 0;
					} else {
						fx[fcount] = x;
						fy[fcount] = y;
						fcount++;
					}
				} else {
					gestx[skip] = x;
					gesty[skip] = y;
				}
			} else {
				if(skip != 0) {
					trygesture();

					skip = 0;
					fx[fcount] = -1024;
					fy[fcount] = -1024;
					fcount++;
				}
				
			}
		}
	
	
	
	    /* handle the events in the queue */

	    while(SDL_PollEvent(&event)) {
		    switch(event.type) {
			
			case SDL_MOUSEMOTION:
				//SDL_GetMouseState(&curx, &cury);
				break;
			
			
			case SDL_MOUSEBUTTONUP:
				switch(event.button.button) {
					case SDL_BUTTON_LEFT:
						break;
				
					case SDL_BUTTON_WHEELUP:
                        break;

					case SDL_BUTTON_WHEELDOWN:
						break;
				}
				break;
			
			case SDL_ACTIVEEVENT:
			    /* Something's happend with our focus
			     * If we lost focus or we are iconified, we
			     * shouldn't draw the screen
			     */
			    if ( event.active.gain == 0 )
				isActive = TRUE; //FALSE;
			    else
				isActive = TRUE;
			    break;			    
			case SDL_VIDEORESIZE:
			    /* handle resize event */
			    surface = SDL_SetVideoMode( event.resize.w,
							event.resize.h,
							16, videoFlags );
			    if ( !surface )
				{
				    fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
				    Quit( 1 );
				}
			    resizeWindow( event.resize.w, event.resize.h );
			    break;
			case SDL_KEYDOWN:
			    /* handle key presses */
			    handleKeyPress( &event.key.keysym );
			    break;
			case SDL_QUIT:
			    /* handle quit requests */
			    done = TRUE;
			    break;
			default:
			    break;
			}
		}

	    /* draw the scene */
	    if ( isActive )
		drawGLScene( );
	}

    /* clean ourselves up and exit */
    Quit( 0 );

    /* Should never get here */
    return( 0 );
}




