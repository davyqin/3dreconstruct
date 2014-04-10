#ifndef VIEW3DDIALOG_H
#define VIEW3DDIALOG_H

#include <QDialog>
#include <memory>

class View3DDialog : public QDialog
{
  Q_OBJECT

public:
  explicit View3DDialog(QWidget *parent = 0);
  ~View3DDialog();

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};

#endif // VIEW3DDIALOG_H
