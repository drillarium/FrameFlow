#include "settingsdialog.h"
#include <QKeyEvent>

SettingsDialog::SettingsDialog(QWidget *_parent)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Escape)
  {
    event->ignore();
    return;
  }
  QDialog::keyPressEvent(event);
}