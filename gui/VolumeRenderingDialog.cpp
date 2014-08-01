#include "VolumeRenderingDialog.h"
#include "model/Image.h"

#include "ui_VolumeRenderingDialog.h"

class VolumeRenderingDialog::Pimpl
{
public:
  Pimpl() {}

  /* data */
  Ui::VolumeRenderingDialog ui;
};

VolumeRenderingDialog::VolumeRenderingDialog(QWidget *parent) 
:QDialog(parent, Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint)
,_pimpl(new Pimpl())
{
  _pimpl->ui.setupUi(this);

  _pimpl->ui.vrWidget->setFocusPolicy(Qt::StrongFocus);

  setWindowTitle(tr("Volume Rendering"));

  connect(_pimpl->ui.show3DButton, SIGNAL(clicked()), SIGNAL(show3DSignal()));
  connect(_pimpl->ui.isoButton, SIGNAL(clicked()), SLOT(onApplyISO()));
}

VolumeRenderingDialog::~VolumeRenderingDialog() {}

void VolumeRenderingDialog::show3D(std::vector<boost::shared_ptr<const Image> >& images) {
  if (images.empty()) return;
  _pimpl->ui.vrWidget->setImages(images);
  _pimpl->ui.vrWidget->update();
}

void VolumeRenderingDialog::onApplyISO() {
  _pimpl->ui.vrWidget->setISO(_pimpl->ui.minIsoSpinBox->text().toInt(),
                              _pimpl->ui.maxIsoSpinBox->text().toInt());

  _pimpl->ui.vrWidget->update();
}
