#include "View3DDialog.h"
#include "ui_View3DDialog.h"

#include "mc/Triangle.h"

using namespace std;

class View3DDialog::Pimpl
{
public:
  Pimpl() {}

  /* data */
  Ui::View3DDialog ui;
};

View3DDialog::View3DDialog(QWidget *parent)
:QDialog(parent, Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint) 
,_pimpl(new Pimpl())
{
  _pimpl->ui.setupUi(this);

  setWindowTitle(tr("View 3D Reconstruction"));
}

View3DDialog::~View3DDialog() {}

void View3DDialog::show3D(const std::vector<boost::shared_ptr<const Triangle> >& data) {
  _pimpl->ui.d3Widget->setData(data);
  _pimpl->ui.d3Widget->update();
}
