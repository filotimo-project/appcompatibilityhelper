Assuming you have a KDE development environment set up, you can build and install this with the following:
```
cmake -B build/ -DCMAKE_INSTALL_PREFIX=~/kde/usr && cmake --build build/ -v && cmake --install build/
```