#include <QtGui>
#include <QtOpenGL>

#include <math.h>

#include "GLWidget.h"
#include <GL/glu.h>
#include <GL/glut.h>

#include <iostream>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

namespace {
  unsigned char* pData = 0;
}

//! [0]
GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::DoubleBuffer), parent)
{
    qtRed = QColor::fromRgb(255,0,0);
    qtDark = QColor::fromRgb(0,0,0);
    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

    dicomUtil.setFileName("./ct.dcm");
    pData = dicomUtil.pixel();
}
//! [0]

//! [1]
GLWidget::~GLWidget()
{
  delete [] pData;
}
//! [1]

//! [2]
QSize GLWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}
//! [2]

//! [3]
QSize GLWidget::sizeHint() const
//! [3] //! [4]
{
    return QSize(500, 500);
}
//! [4]


//! [6]
void GLWidget::initializeGL()
{
    qglClearColor(qtPurple);
//    qglColor(qtRed); /* draw in red */

//    glEnable(GL_DEPTH_TEST);
//    glShadeModel(GL_SMOOTH);
//    glEnable(GL_MULTISAMPLE);
}
//! [6]

//! [7]
void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glDrawPixels(dicomUtil.imageWidth(), dicomUtil.imageHeight(), GL_LUMINANCE, GL_UNSIGNED_BYTE, pData);
 #if 0
    qglColor(qtRed); /* draw in red */

    /* define a point data type */

    typedef GLfloat point2[2];

    point2 vertices[3]={{0.0,0.0},{250.0,500.0},{500.0,0.0}}; /* A triangle */

    int j, k;
    int rand();       /* standard random number generator */
    point2 p ={75.0,50.0};  /* An arbitrary initial point inside traingle */

    glClear(GL_COLOR_BUFFER_BIT);  /*clear the window */


    /* compute and plots 5000 new points */

    for( k=0; k<10000; k++)
    {
        j=rand()%3; /* pick a vertex at random */


        /* Compute point halfway between selected vertex and old point */

        p[0] = (p[0]+vertices[j][0])/2.0;
        p[1] = (p[1]+vertices[j][1])/2.0;

        /* plot new point */

        glBegin(GL_POINTS);
        glVertex2fv(p);
        glEnd();

    }
#endif
    glFlush(); /* clear buffers */

}
//! [7]

//! [8]
void GLWidget::resizeGL(int width, int height)
{
#if 1
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //gluOrtho2D(0.0, 500.0, 0.0, 500.0);
    glOrtho(0.0, 500.0, 0.0, 500.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
#endif
}
//! [8]
