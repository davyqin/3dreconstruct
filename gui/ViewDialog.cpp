#include "ViewDialog.h"
#include "model/Image.h"

#include "ui_ViewDialog.h"

#include <QFileDialog>


class ViewDialog::Pimpl
{
public:
  Pimpl() {}

  /* data */
  Ui::ViewDialog ui;
  QString imageFolder;
};

ViewDialog::ViewDialog(QWidget *parent) 
:QDialog(parent, Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint)
,_pimpl(new Pimpl())
{
  _pimpl->ui.setupUi(this);

  setWindowTitle(tr("3D Reconstruct"));

  connect(_pimpl->ui.browseButton, SIGNAL(clicked()), SLOT(onBrowseFolder()));
  connect(_pimpl->ui.loadButton, SIGNAL(clicked()), SLOT(onLoadImage()));
  connect(_pimpl->ui.imagePosSlider, SIGNAL(valueChanged(int)), SIGNAL(requestImage(int)));
  connect(_pimpl->ui.glWidget, SIGNAL(requestNextImage()), SLOT(onNextImage()));
  connect(_pimpl->ui.glWidget, SIGNAL(requestPrevImage()), SLOT(onPrevImage()));
  connect(_pimpl->ui.applyWLButton, SIGNAL(clicked()), SLOT(onApplyWL()));
  connect(_pimpl->ui.mcButton, SIGNAL(clicked()), SIGNAL(show3dSignal()));
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

void ViewDialog::onNextImage() {
  const int currentValue = _pimpl->ui.imagePosSlider->value();
  if (currentValue < _pimpl->ui.imagePosSlider->maximum()) {
    _pimpl->ui.imagePosSlider->setValue(currentValue + 1);
  }
}

void ViewDialog::onPrevImage() {
  const int currentValue = _pimpl->ui.imagePosSlider->value();
  if (currentValue > _pimpl->ui.imagePosSlider->minimum()) {
    _pimpl->ui.imagePosSlider->setValue(currentValue - 1);
  }
}

void ViewDialog::onApplyWL() {
  emit(updateWLSignal(_pimpl->ui.windowSpinBox->text().toInt(),
                      _pimpl->ui.levelSpinBox->text().toInt()));
}

void ViewDialog::show3D(const std::vector<Triangle>& data) {
  _pimpl->ui.glWidget->set3dData(data);
  _pimpl->ui.glWidget->update();
}