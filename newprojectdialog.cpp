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

void NewProjectDialog::onCreateProject()
{
  project_.name = ui.lineEdit->text();
  accept();
}