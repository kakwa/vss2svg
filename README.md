vss2svg
=======

Visio stencils converter to svg

[![Build Status](https://travis-ci.org/kakwa/vss2svg.svg?branch=master)](https://travis-ci.org/kakwa/vss2svg)

Dependencies
============

* [librevenge](http://sourceforge.net/p/libwpd/wiki/librevenge/)
* [libvisio](https://github.com/LibreOffice/libvisio)
* [libemf2svg](https://github.com/kakwa/libemf2svg)

Building
========

Commands to build this project:

```bash
# CMAKE_INSTALL_PREFIX is optional, default is /usr/local/
$ cmake . -DCMAKE_INSTALL_PREFIX=/usr/

# compilation
$ make

# installation
$ make install
```

Usage
=====

Command Line
------------

```bash
# help
$ ./vss2svg-conv --help
Usage: vss2svg-conv [OPTION...] [options] -i <in vss> -o <out dir>
vss2svg -- Visio stencil to SVG converter

  -i, --input=FILE           Input Visio .vss file
  -o, --output=FILE/DIR      Output file (yED) or directory (svg)
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
      --version              Print program version
  -V, --version              Print vss2svg version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <carpentier.pf@gmail.com>.

# conversion
$ ./vss2svg-conv -i ./tests/resources/vss/2960CX.vss -o ./tests/out/
```

Library
-------

```cpp
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <librevenge-stream/librevenge-stream.h>
#include <librevenge-generators/librevenge-generators.h>
#include <librevenge/librevenge.h>
#include <libvisio/libvisio.h>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SVGDrawingGenerator.h>

int main() {
    librevenge::RVNGFileStream input("in.vss");
    librevenge::RVNGStringVector output;
    vss2svg::SVGDrawingGenerator generator(output, NULL);
    if (!libvisio::VisioDocument::parseStencils(&input, &generator)) {
        std::cerr << "ERROR: SVG Generation failed!" << std::endl;
        return 1;
    }
    if (output.empty()) {
        std::cerr << "ERROR: No SVG document generated!" << std::endl;
        return 1;
    }
    std::string outputdir("outdir");
    mkdir("outdir", S_IRWXU);
    for (unsigned k = 0; k < output.size(); ++k) {
        ofstream myfile;

        std::basic_string<char> newfilename =
            outputdir + "/image-" + std::to_string(k) + ".svg";
        myfile.open(newfilename);
        myfile << output[k].cstr() << std::endl;
        myfile.close();
    }
}
```
