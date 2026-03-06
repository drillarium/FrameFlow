#include "effectsdialog.h"

EffectsDialog::EffectsDialog(QWidget *parent)
:QDialog(parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);
}

EffectsDialog::~EffectsDialog()
{

}
