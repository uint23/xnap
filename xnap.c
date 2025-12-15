#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include "config.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

enum mode {
	MODE_SEL,
	MODE_FLL,
	MODE_SCR,
	MODE_WIN
};

struct pointer_t {
	int    ret;

	Bool   sel;
	Cursor cur;

	int    x0;
	int    y0;
	int    x1;
	int    y1;
	int    lx;
	int    ly;
};

void capfull(void);
void capscr(void);
void capsel(void);
void capwin(void);
unsigned char channel(unsigned long px, Mask m);
void die(const char* s);
void drawrect(void);
void mkppm(XImage* img);
void parseargs(int argc, char** argv);
void quit(Bool ex);
void run(void);
void setup(void);

Display* dpy = NULL;
Window root = None;
GC selgc = 0;
int scr = -1;

enum mode mode = MODE_SEL;
int selscr = -1;
struct pointer_t p = {0};

void capfull(void)
{
	int w = DisplayWidth(dpy, scr);
	int h = DisplayHeight(dpy, scr);

	XImage* img = XGetImage(dpy, root, 0, 0, w, h, AllPlanes, ZPixmap);
	if (!img)
		die("XGetImage failed");

	mkppm(img);
}

void capscr(void)
{
#ifdef XINERAMA
	int n;
	XineramaScreenInfo* si = XineramaQueryScreens(dpy, &n);
	if (!si || selscr < 0 || selscr >= n) {
		if (si)
			XFree(si);
		die("invalid screen");
	}

	int x = si[selscr].x_org;
	int y = si[selscr].y_org;
	int w = si[selscr].width;
	int h = si[selscr].height;
	XFree(si);

	XImage* img = XGetImage(dpy, root, x, y, w, h, AllPlanes, ZPixmap);
	if (!img)
		die("XGetImage failed");

	mkppm(img);
#else
	die("xinerama support not compiled");
#endif
}

void capsel(void)
{
	int rx = MIN(p.x0, p.x1);
	int ry = MIN(p.y0, p.y1);
	int rw = MAX(p.x0, p.x1) - rx;
	int rh = MAX(p.y0, p.y1) - ry;

	if (rw <= 0 || rh <= 0)
		die("empty selection");

	XImage* img = XGetImage(dpy, root, rx, ry, rw, rh, AllPlanes, ZPixmap);
	if (!img)
		die("XGetImage failed");

	mkppm(img);
}

void capwin(void)
{
	Window child;
	Window rroot;
	int rx;
	int ry;
	int wx;
	int wy;
	unsigned int rw;
	unsigned int rh;
	unsigned int rb;
	unsigned int rd;

	/* find window under pointer */
	XQueryPointer(dpy, root, &rroot, &child, &rx, &ry, &wx, &wy, &rb);
	if (child == None)
		child = root;

	XGetGeometry(dpy, child, &rroot, &rx, &ry, &rw, &rh, &rb, &rd);
	XTranslateCoordinates(dpy, child, root, 0, 0, &rx, &ry, &rroot);

	XImage* img = XGetImage(dpy, root, rx, ry, rw, rh, AllPlanes, ZPixmap);
	if (!img)
		die("XGetImage failed");

	mkppm(img);
}

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

void die(const char* s)
{
	fprintf(stderr, "xnap: %s\n", s);
	exit(EXIT_FAILURE);
}

void drawrect(void)
{
	int rx = MIN(p.x0, p.x1);
	int ry = MIN(p.y0, p.y1);
	int rw = MAX(p.x0, p.x1) - rx;
	int rh = MAX(p.y0, p.y1) - ry;

	if (rw > 0 && rh > 0)
		XDrawRectangle(dpy, root, selgc, rx, ry, rw, rh);
}

void mkppm(XImage* img)
{
	FILE* out = stdout;

	/* write ppm metadata header */
	fprintf(out, "P6\n%d %d\n255\n", img->width, img->height);

	for (int y = 0; y < img->height; y++) {
		for (int x = 0; x < img->width; x++) {
			unsigned long px = XGetPixel(img, x, y);
			fputc(channel(px, img->red_mask), out);
			fputc(channel(px, img->green_mask), out);
			fputc(channel(px, img->blue_mask), out);
		}
	}

	fflush(stdout);
}

void parseargs(int argc, char** argv)
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-f"))
			mode = MODE_FLL;

		else if (!strcmp(argv[i], "-w"))
			mode = MODE_WIN;

		else if (!strcmp(argv[i], "-s") && i+1 < argc) {
			mode = MODE_SCR;
			selscr = atoi(argv[++i]);
		}
		else
			die("usage: xnap [-f | -w | -s N]");
	}
}

void quit(Bool ex)
{
	XUngrabPointer(dpy, CurrentTime);
	XFreeCursor(dpy, p.cur);
	XFreeGC(dpy, selgc);
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
		if (ev.type == ButtonPress && b == Button1) {
			if (mode == MODE_WIN) {
				capwin();
				quit(True);
			}
			/* start selecting for selection mode */
			p.sel = True;
			p.x0 = p.x1 = p.lx = ev.xbutton.x_root;
			p.y0 = p.y1 = p.ly = ev.xbutton.y_root;
		}
		else if (ev.type == MotionNotify && p.sel) { /* dragging selection */
			/* remove old rectangle */
			drawrect();

			p.x1 = ev.xmotion.x_root;
			p.y1 = ev.xmotion.y_root;

			/* make new rect */
			drawrect();

			p.lx = p.x1;
			p.ly = p.y1;
		}
		else if (ev.type == ButtonRelease && p.sel && b == Button1) { /* release selection */
			/* remove final rectangle */
			drawrect();

			p.sel = False;
			capsel();
			quit(True);
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
	const unsigned int cur = (mode == MODE_WIN ? win_cursor : sel_cursor);
	p.cur = XCreateFontCursor(dpy, cur);
	p.ret = XGrabPointer(
		dpy, root, False,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		GrabModeAsync, GrabModeAsync, None, p.cur, CurrentTime
	);
	if (p.ret != GrabSuccess)
		die("XGrabPointer failed");

	/* selection rectangle */
	XGCValues gcv;
	gcv.function = GXxor;
	gcv.foreground = WhitePixel(dpy, scr) ^ BlackPixel(dpy, scr);
	gcv.line_style = LineSolid;
	gcv.line_width = sel_line_width;
	gcv.subwindow_mode = IncludeInferiors;

	selgc = XCreateGC(dpy, root, GCFunction | GCForeground | GCLineWidth | GCLineStyle | GCSubwindowMode, &gcv);
}

int main(int argc, char** argv)
{
	parseargs(argc, argv);

	/* immediate capture modes */
	if (mode == MODE_FLL || mode == MODE_SCR) {
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL)
			die("failed to open display");
		scr = DefaultScreen(dpy);
		root = RootWindow(dpy, scr);

		if (mode == MODE_FLL)
			capfull();
		else
			capscr();

		XCloseDisplay(dpy);
		return EXIT_SUCCESS;
	}

	setup();
	run();
	quit(True); /* unreachable */

	return EXIT_SUCCESS;
}
