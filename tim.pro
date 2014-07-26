QT       += core gui

TARGET = tim
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    Arguments.cpp \
    TimFile.cpp \
    TextureFile.cpp \
    TexFile.cpp \
    TextureImageFile.cpp \
    PsColor.cpp \
    ExtraData.cpp \
    tests/Collect.cpp

HEADERS += \
    Arguments.h \
    TimFile.h \
    TextureFile.h \
    TexFile.h \
    TextureImageFile.h \
    PsColor.h \
    ExtraData.h \
    tests/Collect.h

OTHER_FILES += README.md
