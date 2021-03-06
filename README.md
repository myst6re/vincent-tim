Vincent Tim (Twentycen Ts)
==========================

Command-line tool to convert texture files from Final Fantasy VII and VIII (tim and tex formats).

Features
--------

The main purpose of this software is to export and reimport a texture image
to be able to modify this texture with an image editor.

But it is also possible to search and extract tim files from a file, and
convert tim files to tex files and vice versa.

Usage
-----

### Export a texture to image files

    tim foo.tim output_directory
    tim bar.tex output_directory

You can also specify input/output formats:

    tim --if tex --of bmp bar.tex output_directory

If you want to import your images later, you can export some data that are
not exportable in a simple image file.

    tim -e bar.tex output_directory # The 'e' flag export all extra data
    tim --export-palette --export-meta bar.tex output_directory # You can use specific flags to do the same thing

### Import png file to a texture

You do not need to import all the exported images, just the modified one, and
you must specify (with -p) the palette number used for this image (it's indicated in the filename).
But to import you must have exported the palette and the meta data with the "-e" flag.

    tim --if png --of tim \
        -p 1 foo.tim.1.png output_directory

This previous command automatically search palette and meta files
in foo.tim.palette.png and foo.tim.meta, but you can set custom
paths like this:

    tim --if png --of tim \
        -p 1 \
        --input-path-palette foo.tim.palette.png \
        --input-path-meta foo.tim.meta \
        foo.tim.1.png output_directory

### Convert a texture to another

    tim --if tex --of tim \
        --input-path-meta bar.meta \
        bar.tex output_directory

When creating tex files, the meta data should contains at least these values:

    # Tex version (1=FF7, 2=FF8)
    version=1
    # 1 if the image contains transparency
    hasAlpha=0

(Many) other values are possible, but common cases are auto-generated
with these few values.

When creating tim files, here is the basic meta data file:

    paletteX=0
    paletteY=0
    imageX=0
    imageY=0

These are the coordinates where the texture is copied in PlayStation VRAM.

### Extract tim files from an archive

    tim -a --of png archive.foo output_directory
    tim -a --of tim archive.foo output_directory
