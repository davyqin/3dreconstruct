#ifndef VIEWDIALOG_H
#define VIEWDIALOG_H

#include <QDialog>

#include <memory>

#include <boost/shared_ptr.hpp>

class Image;

class ViewDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ViewDialog(QWidget* parent = 0);
  ~ViewDialog();

  void setImageCount(int count);

  void showImage(boost::shared_ptr<const Image> image);

signals:
  void loadImageSignal(const QString& imageFolder);
  void requestImage(int index);
  void requestNextImage();
  void requestPrevImage();

private slots:
  void onBrowseFolder();
  void onLoadImage();

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;

};

#endif // VIEWDIALOG_H
