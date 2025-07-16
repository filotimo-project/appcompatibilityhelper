# App Compatibility Support

Provides support for running or finding alternatives to certain package types on ublue-based distributions.

Utilises Zorin's database for matching Windows executables for Flatpaks, and extracts AppStream metainfo from .rpm and .deb packages to match those to Flatpaks. 
If one can't be matched, it shows a generic message telling the user what to do. In the case of Windows executables, it shows an option to install or run Bottles (and in future, a few configurable choices of Wine layers).

Extensible for any mimetype - just implement `ICompatibilityHelper` and add a case to `CompatibilityHelperFactory`.

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
