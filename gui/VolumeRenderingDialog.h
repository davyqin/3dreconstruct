#ifndef VOLUMENRENDERINGDIALOG_H
#define VOLUMENRENDERINGDIALOG_H

#include <QDialog>

#include <memory>
#include <vector>

#include <boost/shared_ptr.hpp>

class Image;

class VolumeRenderingDialog : public QDialog
{
  Q_OBJECT
public:
  explicit VolumeRenderingDialog(QWidget* parent = 0);
  ~VolumeRenderingDialog();

  void show3D(std::vector<boost::shared_ptr<const Image> >& images);

signals:
  void show3DSignal();

private slots:
  void onApplyISO();

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;

};

#endif // VOLUMENRENDERINGDIALOG_H
