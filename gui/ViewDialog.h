#ifndef VIEWDIALOG_H
#define VIEWDIALOG_H

#include <QDialog>

#include <memory>

class ViewDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ViewDialog(QWidget* parent = 0);
  ~ViewDialog();

signals:

public slots:

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;

};

#endif // VIEWDIALOG_H
