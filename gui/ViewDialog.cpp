#include "ViewDialog.h"

#include "ui_ViewDialog.h"

#include <QFileDialog>

#include <iostream>

class ViewDialog::Pimpl
{
public:
  Pimpl() {}

  /* data */
  Ui::ViewDialog ui;
  QString imageFolder;
};

ViewDialog::ViewDialog(QWidget *parent) 
:QDialog(parent), _pimpl(new Pimpl())
{
  _pimpl->ui.setupUi(this);

  setWindowTitle(tr("3D Reconstruct"));

  connect(_pimpl->ui.browseButton, SIGNAL(clicked()), SLOT(onBrowseFolder()));
  connect(_pimpl->ui.loadButton, SIGNAL(clicked()), SLOT(onLoadImage()));
}

ViewDialog::~ViewDialog() {}

void ViewDialog::onBrowseFolder() {
  QFileDialog browser(this, tr("Select Image Stack Directory"));
  browser.setFileMode(QFileDialog::DirectoryOnly);
  browser.setAcceptMode(QFileDialog::AcceptOpen);
  if(browser.exec() == QFileDialog::Accepted) {
    _pimpl->imageFolder = browser.selectedFiles().at(0);
    _pimpl->ui.imageFolderLineEdit->setText(_pimpl->imageFolder);
  }
}

void ViewDialog::onLoadImage() {
  if (!_pimpl->imageFolder.isEmpty()) {
    emit(loadImageSignal(_pimpl->imageFolder));
  }
}
