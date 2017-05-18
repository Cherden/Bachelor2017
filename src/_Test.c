#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "libfreenect/libfreenect.h"
#include "libfreenect/libfreenect_sync.h"

#ifndef SIGQUIT // win32 compat
	#define SIGQUIT SIGTERM
#endif
/*void bitmap(char* frame)
{
	typedef struct                       /**** BMP file header structure ****/
    {
	    unsigned int   bfSize;           /* Size of file */
	    unsigned short bfReserved1;      /* Reserved */
	    unsigned short bfReserved2;      /* ... */
	    unsigned int   bfOffBits;        /* Offset to bitmap data */
    } BITMAPFILEHEADER;

	typedef struct                       /**** BMP file info structure ****/
    {
	    unsigned int   biSize;           /* Size of info header */
	    int            biWidth;          /* Width of image */
	    int            biHeight;         /* Height of image */
	    unsigned short biPlanes;         /* Number of color planes */
	    unsigned short biBitCount;       /* Number of bits per pixel */
	    unsigned int   biCompression;    /* Type of compression to use */
	    unsigned int   biSizeImage;      /* Size of image data */
	    int            biXPelsPerMeter;  /* X pixels per meter */
	    int            biYPelsPerMeter;  /* Y pixels per meter */
	    unsigned int   biClrUsed;        /* Number of colors used */
	    unsigned int   biClrImportant;   /* Number of important colors */
    } BITMAPINFOHEADER;

	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;

	/* Magic number for file. It does not fit in the header structure due to alignment requirements, so put it outside */
	unsigned short bfType=0x4d42;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfSize = 2+sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+640*480*3;
	bfh.bfOffBits = 0x36;

	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = 640;
	bih.biHeight = 480;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = 0;
	bih.biSizeImage = 0;
	bih.biXPelsPerMeter = 5000;
	bih.biYPelsPerMeter = 5000;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	FILE *file = fopen("a.bmp", "wb");
	if (!file)
    {
    printf("Could not write file\n");
    return;
    }

	/*Write headers*/
	fwrite(&bfType,1,sizeof(bfType),file);
	fwrite(&bfh, 1, sizeof(bfh), file);
	fwrite(&bih, 1, sizeof(bih), file);

	/*Write bitmap*/
	for (int y = bih.biHeight-1; y>=0; y--) /*Scanline loop backwards*/
    {
    	for (int x = 0; x < bih.biWidth; x++) /*Column loop forwards*/
        {
        /*compute some pixel values*/
        /*unsigned char r = 255*((float)x/bih.biWidth);
        unsigned char g = 255*((float)y/bih.biHeight);
        unsigned char b = 0;
        fwrite(&b, 1, 1, file);
        fwrite(&g, 1, 1, file);*/
        fwrite(&frame[y*x], 1, sizeof(char), file);
        }
    }
	fclose(file);
}*/


volatile bool running = true;
void signalHandler(int signal)
{
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT)
	{
		running = false;
	}
}

char* rgb_raw;
char* depth_registered;

int kinectUpdate(void) {
    uint32_t ts;
	printf("in update\n");

    // Pull a (distorted) RGB frame
    int ret = freenect_sync_get_video_with_res((void **) &rgb_raw, &ts, 0,
	    FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB);
    if (ret != 0) {
	printf("Error: unable to acquire RGB stream\n");
		return ret;
    }

	printf("Got rgb %d\n", ts);

	// Pull a depth frame registered to the above image
	int retD = freenect_sync_get_depth_with_res((void **) &depth_registered, &ts, 0,
					FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED);

	if (retD != 0) {
		printf("Error: unable to acquire registered depth stream\n");
		return retD;
	}

	printf("Got depth %d\n", ts);

	return 0;
}

int main(int argc, char** argv)
{
	// Handle signals gracefully.
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	running = true;
	printf("before while\n");
	// Run until interruption or failure.
	while (running)
	{
		kinectUpdate();
		usleep(1000);
	}

	printf("Shutting down\n");

	// Stop everything and shutdown.
	freenect_sync_stop();

	printf("Done!\n");

	return 0;
}
