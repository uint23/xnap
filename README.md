# xnap

xnap is a minimalistic screenshot utility for X.  
just `make install`

you **need** `Xlib` and can use `Xinerama` optionally.  
xnap is minimal and just outputs the image contents (PPM format) to the stdout.  
please use a tool to collect that info and store / convert it to whatever you like.

enjoy!

> example usage
> sxwm: `bind : mod + shift + s : "bash -c 'xnap | pnmtopng | tee ~/Pictures/screenshots/$(date +%Y-%m-%d_%H-%M).png | xclip -selection clipboard -t image/png'"`
