#include "createscenedialog.h"

CreateSceneDialog::CreateSceneDialog(QWidget *parent)
:QDialog(parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);
}

CreateSceneDialog::~CreateSceneDialog()
{

}

void CreateSceneDialog::onAccept()
{
  accept();
}