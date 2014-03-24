######################################################################
# Automatically generated by qmake (3.0) Fri Mar 21 10:57:24 2014
######################################################################

TEMPLATE = app
TARGET = 3dreconstruct
INCLUDEPATH += . gui util
QT += core gui opengl
CONFIG += c++11

win32 {
INCLUDEPATH += C:/boost_1_54_0
LIBS += "C:/boost_1_54_0/stage/lib/libboost_system-mgw48-1_54.a"
#LIBS += "C:/boost_1_54_0/stage/lib/libboost_filesystem-mgw48-1_54.a"
}

unix {
INCLUDEPATH += /opt/boost-1.49.0-gcc-4.8.1-static/
LIBS += "-L/opt/boost-1.49.0-gcc-4.8.1-static/lib/ --llibboost_system"
}


# Input
HEADERS += gui/GLWidget.h gui/Window.h util/DicomUtil.h
SOURCES += main.cpp gui/GLWidget.cpp gui/Window.cpp util/DicomUtil.cpp
