#include "ViewDialog.h"
#include "model/Image.h"
#include "cuda/cuda_info.h"

#include "ui_ViewDialog.h"

#include <QFileDialog>
#include <QMessageBox>


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

  _pimpl->ui.glWidget->setFocusPolicy(Qt::StrongFocus);

  setWindowTitle(tr("View Image"));

  connect(_pimpl->ui.browseButton, SIGNAL(clicked()), SLOT(onBrowseFolder()));
  connect(_pimpl->ui.loadButton, SIGNAL(clicked()), SLOT(onLoadImage()));
  connect(_pimpl->ui.imagePosSlider, SIGNAL(valueChanged(int)), SIGNAL(requestImage(int)));
  connect(_pimpl->ui.glWidget, SIGNAL(requestNextImage()), SLOT(onNextImage()));
  connect(_pimpl->ui.glWidget, SIGNAL(requestPrevImage()), SLOT(onPrevImage()));
  connect(_pimpl->ui.applyWLButton, SIGNAL(clicked()), SLOT(onApplyWL()));
  connect(_pimpl->ui.imageTypeCombo, SIGNAL(currentIndexChanged(int)), SLOT(onImageType(int)));
  connect(_pimpl->ui.orientationCombo, SIGNAL(currentIndexChanged(int)), SIGNAL(orientationSignal(int)));
  connect(_pimpl->ui.filterComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onFilter(int)));
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

void ViewDialog::onImageType(int index) {
  _pimpl->ui.applyWLButton->setEnabled(index == 0);
  _pimpl->ui.filterComboBox->setEnabled(index == 1);
  _pimpl->ui.glWidget->setDataType(index);
  emit(requestImage(_pimpl->ui.imagePosSlider->value()));
}

void ViewDialog::onFilter(int filter) {
  if (filter == 2 && cudaAvaliable() == 0) {
    QMessageBox::warning(this, tr("CUDA Warning"),
                               tr("NO CUDA device is avaliable!"),
                                  QMessageBox::Ok, QMessageBox::Ok);
    return;
  }

  _pimpl->ui.glWidget->setFilter(filter);
}
