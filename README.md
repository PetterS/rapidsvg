I have sometimes had to open very large SVG files, which is slow in Inkscape and any other program I have tried. RapidSVG is much faster than Inkscape to open and render a file, but can only handle lines of various sizes and colors.

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
Use CMake.
```
sudo apt-get install freeglut3 freeglut3-dev
sudo apt-get install binutils-gold
sudo apt-get install libxmu-dev libxi-dev
```
