#include <stdlib.h>
#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

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

void compimg(void);
void die(const char* s);
void quit(Bool ex);
void run(void);
void setup(void);

Display* dpy = NULL;
Window root = None;
int scr = -1;
struct pointer_t p = {0};
XImage* img = NULL;

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

	unsigned long p0 = XGetPixel(img, 0, 0);
	fprintf(stderr, "first pixe =#%lx depth=%d bpp=%d\n", p0, img->depth, img->bits_per_pixel);

	XDestroyImage(img);
	quit(True);
}

void die(const char* s)
{
	fprintf(stderr, "xnap: %s", s);
	exit(EXIT_FAILURE);
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

		if (ev.type == KeyPress) {
			KeySym ks = XLookupKeysym(&ev.xkey, 0);
			if (ks == XK_Escape || ks == XK_q)
				quit(True);
		}

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
