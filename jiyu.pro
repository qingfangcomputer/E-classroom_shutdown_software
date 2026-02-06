QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    help.cpp \
    killprocessthread.cpp \
    main.cpp \
    mainwindow.cpp \
    progresswindow.cpp \
    stop.cpp \
    up.cpp \
    versionchecker.cpp

HEADERS += \
    help.h \
    killprocessthread.h \
    mainwindow.h \
    progresswindow.h \
    stop.h \
    up.h \
    versionchecker.h

FORMS += \
    help.ui \
    mainwindow.ui \
    stop.ui \
    up.ui

RC_ICONS = logo.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    tupian/tupian.qrc

DISTFILES += \
    tupian/icon file.ico
