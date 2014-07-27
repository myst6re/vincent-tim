Vincent Tim (Twentycen Ts)
==========================

Command-line tool to convert texture files from Final Fantasy VII and VIII (tim and tex formats).

Features
--------

The main purpose of this software is to export and reimport a texture image
to be able to modify this texture with a image editor software.

But it is also possible to search and extract tim files from a file, and
convert tim files to tex files and vice versa.

Usage
-----

### Export a texture to image files

    tim -d output_directory foo.tim
    tim -d output_directory bar.tex

You can also specify input/output formats:

    tim -d output_directory --if tex --of bmp bar.tex

If you want to import your images later, you can export some data that are
not exportable in a simple image file.

    tim -ed output_directory bar.tex # The 'e' flag export all extra data
    tim -d output_directory --export-palette --export-meta bar.tex # You can use specific flags to do the same thing

### Import png file to a texture

You do not need to import all the exported images, just the modified one. But to
import you must have exported the palette and the meta data.

    tim -d output_directory --if png --of tim \
        --input-path-palette foo.palette.png \
        --input-path-meta foo.meta \
        foo.1.png

### Convert a texture to another

    tim -d output_directory --if tex --of tim \
        --input-path-meta bar.meta \
        bar.tex
