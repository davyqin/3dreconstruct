#include "ViewDialog.h"
//#include "GLWidget.h"

#include "ui_ViewDialog.h"

class ViewDialog::Pimpl
{
public:
  Pimpl() {}

  /* data */
  Ui::ViewDialog ui;
};

ViewDialog::ViewDialog(QWidget *parent) 
:QDialog(parent), _pimpl(new Pimpl())
{
  _pimpl->ui.setupUi(this);

  setWindowTitle(tr("3D Reconstruct"));
}

ViewDialog::~ViewDialog() {}
