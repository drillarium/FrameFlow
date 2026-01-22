#include "newprojectdialog.h"

NewProjectDialog::NewProjectDialog(QWidget *parent)
:QDialog(parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);
}

NewProjectDialog::~NewProjectDialog()
{
}


