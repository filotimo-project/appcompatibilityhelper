# Build Instructions

In a container:
```bash
just build
just install
```

Or, assuming you have a KDE development environment container set up that you can log in to the session for, you can build and install this with the following:
```
cmake -B build/ -DCMAKE_INSTALL_PREFIX=~/kde/usr && cmake --build build/ -v && cmake --install build/
```
