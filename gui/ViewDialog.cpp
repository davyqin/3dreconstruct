#include "ViewDialog.h"
#include "model/Image.h"

#include "ui_ViewDialog.h"

#include <QFileDialog>
#include <QIntValidator>


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
  connect(_pimpl->ui.imagePosSlider, SIGNAL(valueChanged(int)), SIGNAL(requestImage(int)));
  connect(_pimpl->ui.glWidget, SIGNAL(requestNextImage()), SIGNAL(requestNextImage()));
  connect(_pimpl->ui.glWidget, SIGNAL(requestPrevImage()), SIGNAL(requestPrevImage()));
  connect(_pimpl->ui.applyWLButton, SIGNAL(clicked()), SLOT(onApplyWL()));

  QValidator *validator = new QIntValidator(0, 65535, this);
  _pimpl->ui.windowLineEdit->setValidator(validator);
  _pimpl->ui.levelLineEdit->setValidator(validator);
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

void ViewDialog::onApplyWL() {
  emit(updateWLSignal(_pimpl->ui.windowLineEdit->text().toInt(),
                      _pimpl->ui.levelLineEdit->text().toInt()));
}