#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

#include "util/DicomUtil.h"

//! [0]
class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
//! [0]

#if 0
//! [1]
public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
//! [1]
#endif

//! [2]
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
//! [2]

//! [3]
private:
    QColor qtRed;
    QColor qtDark;
    QColor qtPurple;

    DicomUtil dicomUtil;
};
//! [3]

#endif
