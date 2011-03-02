BUILD = ../build
SOURCES = shmlaunch.c
TEMPLATE = app
TARGET = shmlaunch
CONFIG -= qt
CONFIG += use_c_linker

!include( ../buildpath.pri ) { error("Unable to find build path specification") }