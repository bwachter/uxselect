BUILD = ../build
SOURCES = pamhelper.c
TEMPLATE = app
TARGET = uxselect.pamhelper
CONFIG -= qt
CONFIG += use_c_linker
QMAKE_CFLAGS += -std=gnu99
LIBS += -lpam

unix:target.path = $${PREFIX}/${PREFIX}/bin
INSTALLS += target

!include( ../buildpath.pri ) { error("Unable to find build path specification") }