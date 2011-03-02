contains(QT_VERSION, ^4\.[0-5]\..*) {
    error("Can't build with Qt version $${QT_VERSION}. Use at least Qt 4.6.")
}

TEMPLATE = subdirs
SUBDIRS = src \
        shmlaunch