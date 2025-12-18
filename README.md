# xnap

xnap is a minimal screenshot utility for X11.

It captures screen contents and writes a raw PPM (P6) image to standard
output. xnap does not save files by itself and does not perform image
conversion.

## Building and installing

```
make
sudo make install
```

## Dependencies

- Xlib (required)
- Xinerama (optional, for multi-monitor support)

## Usage

xnap always writes image data to stdout. Redirect the output to a file or
pipe it to another program for conversion or storage.

Examples:

# Select a region and save as PPM
xnap > image.ppm

# Capture the full screen
xnap -f > fullscreen.ppm

# Capture screen 0 (Xinerama)
xnap -s 0 > screen0.ppm

# Convert to PNG using ImageMagick
xnap | convert ppm:- image.png

## Example key binding

Using sxwm:

bind : mod + shift + s : "bash -c 'xnap | pnmtopng | tee ~/Pictures/screenshots/$(date +%Y-%m-%d_%H-%M).png | xclip -selection clipboard -t image/png'"

## Philosophy

xnap follows the Unix philosophy:
- do one thing
- do it simply
- leave storage and conversion to other tools

Enjoy!

