#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "mc/Triangle.h"
#include <QGLWidget>
#include <QGLFunctions>

#include <memory>
#include <boost/shared_ptr.hpp>

class Image;
class Triangle;

//! [0]
class GLWidget : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setImage(boost::shared_ptr<const Image> image);
    void setDataType(int dataType);

signals:
    void requestNextImage();
    void requestPrevImage();

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
    void wheelEvent(QWheelEvent * event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
//! [2]

//! [3]
private:
    class Pimpl;
    std::unique_ptr<Pimpl> _pimpl;

    void initShaders();
    void initScene();
};
//! [3]

#endif
