#include <stdlib.h>
#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#include "config.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

struct pointer_t {
	int    ret;
	Bool   sel;
	int    x0;
	int    y0;
	int    x1;
	int    y1;
};

unsigned char channel(unsigned long px, Mask m);
void compimg(void);
void die(const char* s);
void mkppm(const char* path, XImage* img);
void quit(Bool ex);
void run(void);
void setup(void);

Display* dpy = NULL;
Window root = None;
int scr = -1;
struct pointer_t p = {0};
XImage* img = NULL;

unsigned char channel(unsigned long px, Mask m)
{
	if (m == 0)
		return 0;

	int shift = 0;
	/* shift down until we find channel */
	while ((m & 1UL) == 0) {
		m >>= 1UL;
		shift++;
	}

	/* bits in channel */
	int bits = 0;
	while (m & 1UL) {
		m >>= 1UL;
		bits++;
	}

	/* raw channel value */
	unsigned long rv = (px >> shift) & ((1UL << bits) - 1UL);

	/* scale v from [0, 2^bits-1]->[0,255] to meet standard */
	if (bits == 8)
		return rv;
	return ((rv * 255UL) / ((1UL << bits) - 1UL));
}

void compimg(void)
{
	/* create selection rectangle */
	int rx = MIN(p.x0, p.x1);
	int ry = MIN(p.y0, p.y1);
	int rw = MAX(p.x0, p.x1) - rx;
	int rh = MAX(p.y0, p.y1) - ry;

	img = XGetImage(dpy, root, rx, ry, rw, rh, AllPlanes, ZPixmap);
	if (!img)
		die("XGetImage failed");

	mkppm("img.ppm", img);
	XDestroyImage(img);
	quit(True);
}

void die(const char* s)
{
	fprintf(stderr, "xnap: %s\n", s);
	exit(EXIT_FAILURE);
}

void mkppm(const char* path, XImage* img)
{
	FILE* f = fopen(path, "w");
	if (!f)
		die("fopen failed");

	/* write ppm metadata header */
	fprintf(f, "P6\n%d %d\n255\n", img->width, img->height);

	for (int y = 0; y < img->height; y++) {
		for (int x = 0; x < img->width; x++) {
			unsigned long px = XGetPixel(img, x, y);
			unsigned char r = channel(px, img->red_mask);
			unsigned char g = channel(px, img->green_mask);
			unsigned char b = channel(px, img->blue_mask);

			fputc(r, f);
			fputc(g, f);
			fputc(b, f);
		}
	}

	fclose(f);
}

void quit(Bool ex)
{
	XUngrabPointer(dpy, CurrentTime);
	XCloseDisplay(dpy);
	if (ex)
		exit(EXIT_SUCCESS);
}

void run(void)
{
	XEvent ev;
	for (;;) {
		XNextEvent(dpy, &ev);


		unsigned int b = ev.xbutton.button;
		if (ev.type == ButtonPress && b == Button1) { /* start selecting */
			p.sel = True;
			p.x0 = p.x1 = ev.xbutton.x_root;
			p.y0 = p.y1 = ev.xbutton.y_root;
		}
		else if (ev.type == MotionNotify && p.sel) { /* dragging selection */
			p.x1 = ev.xmotion.x_root;
			p.y1 = ev.xmotion.y_root;
		}
		else if (ev.type == ButtonRelease && p.sel && b == Button1) { /* release selection */
			p.sel = False;
			compimg();
		}
		else if (ev.type == ButtonPress && (b == Button2 || b == Button3)) { /* quit */
			quit(True);
		}
	}
}

void setup(void)
{
	/* X surface */
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		die ("failed to open display");

	scr = DefaultScreen(dpy);
	root = RootWindow(dpy, scr);

	/* pointer */
	p.ret = XGrabPointer(
		dpy, root, False,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		GrabModeAsync, GrabModeAsync, None, None, CurrentTime
	);
	if (p.ret != GrabSuccess)
		die("XGrabPointer failed");
}

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	setup();
	run();
	quit(True); /* unreachable */

	return EXIT_SUCCESS;
}
