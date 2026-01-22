#include "alertsdialog.h"
#include <QKeyEvent>

AlertsDialog::AlertsDialog(QWidget *_parent)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);
}

AlertsDialog::~AlertsDialog()
{
}

void AlertsDialog::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Escape)
  {
    event->ignore();
    return;
  }
  QDialog::keyPressEvent(event);
}