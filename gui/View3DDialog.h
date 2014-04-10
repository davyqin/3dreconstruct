#ifndef VIEW3DDIALOG_H
#define VIEW3DDIALOG_H

#include <QDialog>

#include <memory>
#include <boost/shared_ptr.hpp>

class Triangle;

class View3DDialog : public QDialog
{
  Q_OBJECT

public:
  explicit View3DDialog(QWidget *parent = 0);
  ~View3DDialog();

  void show3D(const std::vector<boost::shared_ptr<const Triangle> >& data);

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};

#endif // VIEW3DDIALOG_H
