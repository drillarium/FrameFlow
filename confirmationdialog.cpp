#include "confirmationdialog.h"

#include <QMessageBox>

QString buttonToText(QMessageBox::StandardButton button)
{
  switch(button) {
  case QMessageBox::Ok:            return "OK";
  case QMessageBox::Cancel:        return "Cancel";
  case QMessageBox::Yes:           return "Yes";
  case QMessageBox::No:            return "No";
  case QMessageBox::Abort:         return "Abort";
  case QMessageBox::Retry:         return "Retry";
  case QMessageBox::Ignore:        return "Ignore";
  case QMessageBox::Close:         return "Close";
  case QMessageBox::Open:          return "Open";
  case QMessageBox::Save:          return "Save";
  case QMessageBox::SaveAll:       return "Save All";
  case QMessageBox::Discard:       return "Discard";
  case QMessageBox::Apply:         return "Apply";
  case QMessageBox::Reset:         return "Reset";
  case QMessageBox::RestoreDefaults:return "Restore Defaults";
  case QMessageBox::Help:          return "Help";
  case QMessageBox::YesToAll:      return "Yes to All";
  case QMessageBox::NoToAll:       return "No to All";
  default:
    return "Unknown";
  }
}


ConfirmationDialog::ConfirmationDialog(QWidget *_parent, const QString& _title, const QString& _message, QMessageBox::StandardButton _okButton, QMessageBox::StandardButton _cancelButton, QMessageBox::StandardButton _defaultButton)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  ui.titleLabel->setText(_title);
  ui.plainTextEdit->setPlainText(_message);
  ui.okButton->setText(buttonToText(_okButton));
  ui.cancelButton->setText(buttonToText(_cancelButton));
  okButton_ = _okButton;
  cancelButton_ = _cancelButton;
  if(_defaultButton == _okButton)
  {
    ui.okButton->setDefault(true);
    ui.okButton->setAutoDefault(true);
  }
  else
  {
    ui.cancelButton->setDefault(true);
    ui.cancelButton->setAutoDefault(true);
  }
}

ConfirmationDialog::~ConfirmationDialog()
{
}

QMessageBox::StandardButton ConfirmationDialog::question(QWidget* _parent, const QString& _title, const QString& _message, QMessageBox::StandardButton _okButton, QMessageBox::StandardButton _cancelButton, QMessageBox::StandardButton _defaultButton)
{
  ConfirmationDialog dlg(_parent, _title, _message, _okButton, _cancelButton, _defaultButton);
  dlg.setWindowModality(Qt::ApplicationModal);

  // center
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if(!screen) screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();
  // dlg.adjustSize();
  dlg.move(screenGeometry.center() - dlg.rect().center());

  dlg.exec();

  return dlg.returnValue();
}

QMessageBox::StandardButton ConfirmationDialog::returnValue()
{
  return returnButton_;
}

void ConfirmationDialog::onOk()
{
  accept();
  returnButton_ = okButton_;
}

void ConfirmationDialog::onCancel()
{
  accept();
  returnButton_ = cancelButton_;
}

void ConfirmationDialog::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Escape)
  {
    event->ignore();
    return;
  }

  QDialog::keyPressEvent(event);
}