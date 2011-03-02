BUILD = ../build
FORMS = uxselect.ui
HEADERS = uxselect.h
SOURCES = main.cpp \
        uxselect.cpp
TEMPLATE = app
TARGET = uxselect
LIBS += -lpam

!include( ../buildpath.pri ) { error("Unable to find build path specification") }