#pragma once

#include "mc/Triangle.h"

#include <QGLWidget>

#include <memory>
#include <boost/shared_ptr.hpp>


class Triangle;

//! [0]
class D3Widget : public QGLWidget
{
    Q_OBJECT

public:
    D3Widget(QWidget *parent = 0);
    ~D3Widget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setData(const std::vector<boost::shared_ptr<const Triangle> >& data);

signals:
    void requestNextImage();
    void requestPrevImage();

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
};
//! [3]
