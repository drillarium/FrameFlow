#pragma once

#include <QDialog>
#include <QMessageBox>
#include "ui_confirmationdialog.h"

class ConfirmationDialog : public QDialog
{
Q_OBJECT

public:
  ~ConfirmationDialog();
  static QMessageBox::StandardButton question(QWidget *_parent, const QString &_title, const QString &_message, QMessageBox::StandardButton _okButton, QMessageBox::StandardButton _cancelButton, QMessageBox::StandardButton _defaultButton);

protected:
  ConfirmationDialog(QWidget* _parent, const QString& _title, const QString& _message, QMessageBox::StandardButton _okButton, QMessageBox::StandardButton _cancelButton, QMessageBox::StandardButton _defaultButton);
  QMessageBox::StandardButton returnValue();
  void keyPressEvent(QKeyEvent* event) override;

protected slots:
  void onOk();
  void onCancel();

private:
  Ui::ConfirmationDialogClass ui;
  QMessageBox::StandardButton returnButton_ = QMessageBox::StandardButton::Yes;
  QMessageBox::StandardButton okButton_ = QMessageBox::StandardButton::Yes;
  QMessageBox::StandardButton cancelButton_ = QMessageBox::StandardButton::Yes;
};

