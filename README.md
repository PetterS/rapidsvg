I have sometimes had to open very large SVG files, which is slow in Inkscape and any other program I have tried. RapidSVG is much faster than Inkscape to open and render a file, but can only handle lines of various sizes and colors.

Usage
-----
* Use the mouse to drag the view and the wheel to zoom.
* Press 'R' to reload the file.

Compilation
-----------
Use CMake.
```
mkdir build
cd build
cmake ..
make
```
In Ubuntu, I had to install the following packages:
```
sudo apt-get install freeglut3 freeglut3-dev binutils-gold libxmu-dev libxi-dev
```
