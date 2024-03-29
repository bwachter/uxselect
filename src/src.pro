BUILD = ../build
FORMS = uxselect.ui
HEADERS = uxselect.h
SOURCES = main.cpp \
        uxselect.cpp
RESOURCES = uxselect.qrc
TEMPLATE = app
TARGET = uxselect
LIBS += -lpam
DEFINES += USE_PAMHELPER

unix:target.path = $${PREFIX}/${PREFIX}/bin
INSTALLS += target

!include( ../buildpath.pri ) { error("Unable to find build path specification") }