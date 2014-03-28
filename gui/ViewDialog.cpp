#include "ViewDialog.h"
#include "model/Image.h"

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

  // _pimpl->ui.imagePosSlider->setMinimum(0);
  // _pimpl->ui.imagePosSlider->setValue(0);
  setWindowTitle(tr("3D Reconstruct"));

  connect(_pimpl->ui.browseButton, SIGNAL(clicked()), SLOT(onBrowseFolder()));
  connect(_pimpl->ui.loadButton, SIGNAL(clicked()), SLOT(onLoadImage()));
  connect(_pimpl->ui.imagePosSlider, SIGNAL(valueChanged(int)), SIGNAL(requestImage(int)));
  connect(_pimpl->ui.glWidget, SIGNAL(requestNextImage()), SIGNAL(requestNextImage()));
  connect(_pimpl->ui.glWidget, SIGNAL(requestPrevImage()), SIGNAL(requestPrevImage()));
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

void ViewDialog::setImageCount(int count) {
  _pimpl->ui.imagePosSlider->setMaximum(count - 1);
}

void ViewDialog::showImage(boost::shared_ptr<const Image> image) {
  _pimpl->ui.glWidget->setImage(image);
  _pimpl->ui.glWidget->update();
}
