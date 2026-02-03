#pragma once

#include <QDialog>
#include "ui_settingsdialog.h"

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  SettingsDialog(QWidget *_parent = nullptr);
  ~SettingsDialog();

protected:
  void keyPressEvent(QKeyEvent* event) override;

protected slots:
  void onAddStreamServer();

private:
  Ui::SettingsDialogClass ui;
};

