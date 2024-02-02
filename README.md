## About
Purge is a simple cli tool that recursively *purges* files from
directories or directories themselves from other directories.
It is simple because it is a single C source file under 200 lines 
compliant with C99 standard.

## Example of usage
Best way to explain how purge works is by showing few examples.

This is a file hierarchy before running `purge`:

![file tree before purge](resources/before.png)

---
This command line
```
./purge . goner1
```
or
```
./purge -f . goner1
```

will result in this file hierarchy:

![file tree after file purge](resources/afterfilepurge.png)
---

Additionally running the program with these arguments:
```
./purge -d . deletethis
```

will result in this file hierarchy:

![file tree after directory purge](resources/afterdirpurge.png)
---

If some things are unclear please just run `purge` without arguments,
and check the usage there, cba explaining any further.

## Build

clone the repository and run:
```
make
```

If you wish to build binary with debug symbols enabled then run:
```
make debug
```

if you wish to have more control on how the binary builds just
edit the single `Makefile` to your liking.

---

#### PKGBUILD/AUR
In case you are running Arch-based Linux distribution there is
also `PKGBUILD` provided, build and install like this:
```
makepkg
pacman -U {package}.pkg.tar.zst
```
make sure to replace `{package}` with the actual package name.

Or you can build and install using `pamac` or `yay` because
I am already hosting an AUR package, simply run this in your terminal:
```
pamac build purge
```
or
```
yay -S purge
```
