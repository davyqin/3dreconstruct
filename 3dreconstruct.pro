######################################################################
# Automatically generated by qmake (3.0) Fri Mar 21 10:57:24 2014
######################################################################

TEMPLATE = app
TARGET = 3dreconstruct
INCLUDEPATH += . gui util
QT += core gui opengl 
CONFIG += c++11 console

win32 {
INCLUDEPATH += C:/boost_1_54_0
LIBS += "C:/boost_1_54_0/stage/lib/libboost_system-mgw48-1_54.a"
LIBS += "C:/boost_1_54_0/stage/lib/libboost_filesystem-mgw48-1_54.a"
}

unix {
INCLUDEPATH += /opt/boost-1.49.0-gcc-4.8.1-static/include
LIBS += -L/opt/boost-1.49.0-gcc-4.8.1-static/lib -lboost_system -lboost_filesystem
}


# Input
HEADERS += gui/GLWidget.h \
           util/DicomUtil.h \
           model/Image.h \
           model/ImageStack.h \
           gui/Controller.h \
           gui/ViewDialog.h \
           mc/common.h \
           mc/Vertex.h \
           mc/VertexFactory.h \
           mc/Cube.h \
           mc/CubeFactory.h \
           mc/Grid.h \
           mc/McFactory.h \
           mc/McWorkshop.h       

SOURCES += main.cpp \
   	       gui/GLWidget.cpp \
  	       util/DicomUtil.cpp \
	         model/Image.cpp \
           model/ImageStack.cpp \
	         gui/Controller.cpp \
           gui/ViewDialog.cpp \
           mc/Vertex.cpp \
           mc/VertexFactory.cpp \
           mc/Cube.cpp \
           mc/CubeFactory.cpp \
           mc/Grid.cpp \
           mc/McFactory.cpp \
           mc/McWorkshop.cpp            

MOCABLEHEADERS += gui/ViewDialog.h\

FORMS += \
    gui/ViewDialog.ui
