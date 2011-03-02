BUILD = ../build
FORMS = uxselect.ui
HEADERS = uxselect.h
SOURCES = main.cpp \
        uxselect.cpp
RESOURCES = uxselect.qrc
TEMPLATE = app
TARGET = uxselect
LIBS += -lpam

!include( ../buildpath.pri ) { error("Unable to find build path specification") }