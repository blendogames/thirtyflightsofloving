#!/bin/bash

# flibit has this solely for convenience when cleaning up Mac binaries.

cd "`dirname "$0"`"/tfol
mv kmquake2 tfol
install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib @rpath/libSDL2-2.0.0.dylib tfol
install_name_tool -change libpng16.16.dylib @rpath/libpng16.16.dylib tfol
install_name_tool -change /usr/local/lib/libjpeg.9.dylib @rpath/libjpeg.9.dylib tfol
install_name_tool -change /usr/local/lib/libvorbisfile.3.dylib @rpath/libvorbisfile.3.dylib tfol
install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib @rpath/libSDL2-2.0.0.dylib baseq2/kmq2gamex64.dylib
install_name_tool -change libpng16.16.dylib @rpath/libpng16.16.dylib baseq2/kmq2gamex64.dylib
install_name_tool -change /usr/local/lib/libjpeg.9.dylib @rpath/libjpeg.9.dylib baseq2/kmq2gamex64.dylib
install_name_tool -change /usr/local/lib/libvorbisfile.3.dylib @rpath/libvorbisfile.3.dylib baseq2/kmq2gamex64.dylib
strip -S tfol
strip -S baseq2/kmq2gamex64.dylib
otool -L tfol
otool -L baseq2/kmq2gamex64.dylib
