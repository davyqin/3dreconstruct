#ifndef VOLUMERENDERINGWIDGET_H
#define VOLUMERENDERINGWIDGET_H

#include "mc/Triangle.h"
#include <QGLWidget>
#include <QGLFunctions>

#include <memory>
#include <boost/shared_ptr.hpp>

class Image;

//! [0]
class VolumeRenderingWidget : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT

public:
    VolumeRenderingWidget(QWidget *parent = 0);
    ~VolumeRenderingWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    // void setImage(boost::shared_ptr<const Image> image);

//signals:

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
