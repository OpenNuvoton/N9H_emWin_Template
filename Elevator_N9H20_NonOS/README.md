# Elevator Template on N9H20 NonOS Platform

This is the reference implementation on N9H20 NonOS platform.

### Changelog.pdf

N9H20 Elevator change history.

### Elevator_Reference_Implementation.pdf

N9H20 Elevator user manual.

### Note

Only support Nuvoton demo board "NuDesign HMI-N9H20" + 4.3" 480x272 LCD "NuDesign TFT-LCD4.3".

PNG file support

The PNG (Portable Network Graphics) format is an image format which offers lossless data
compression and alpha blending by using a non-patented data compression method. Version
1.0 of the PNG specification has been released in 1996. Since the end of 2003 PNG
is an international standard (ISO/IEC 15948). PNG support for emWin can be achieved by
using the ’libpng’ library from Glenn Randers-Pehrson, Guy Eric Schalnat and Andreas Dilger.
An adapted version of this library ready to use with emWin is available [on the website](https://www.segger.com/downloads/emwin/emWin_png).
That library can be added to emWin in order to to use the PNG API.

Licensing

The use of ’libpng’ library is subject to a BSD style license and copyright notice in the file
GUI\PNG\png.h of the downloadable library. The original version of the library is available
for free under [www.libpng.org](http://www.libpng.org/).
