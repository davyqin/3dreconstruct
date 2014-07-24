#ifndef VOLUMERENDERINGWIDGET_H
#define VOLUMERENDERINGWIDGET_H

#include <GL/glew.h>

#include <QGLWidget>

#include <memory>
#include <boost/shared_ptr.hpp>

class Image;

//! [0]
class VolumeRenderingWidget : public QGLWidget
{
    Q_OBJECT

public:
    VolumeRenderingWidget(QWidget *parent = 0);
    ~VolumeRenderingWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setImages(std::vector<boost::shared_ptr<const Image> >& images);

//signals:

//! [2]
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
#if 1    
    void wheelEvent(QWheelEvent * event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
#endif    
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
