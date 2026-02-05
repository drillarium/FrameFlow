#include "renamedialog.h"

RenameDialog::RenameDialog(const QString& _name, QWidget *parent)
:QDialog(parent)
{
  ui.setupUi(this);
  ui.lineEdit->setText(_name);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);
}

RenameDialog::~RenameDialog()
{

}
