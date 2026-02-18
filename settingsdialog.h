#pragma once

#include <QDialog>
#include "ui_settingsdialog.h"
#include "server_model.h"

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  SettingsDialog(QWidget *_parent = nullptr);
  ~SettingsDialog();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void addStreamingServer(const StreamingServer& _ss);

protected slots:
  void onAddStreamServer();

private:
  Ui::SettingsDialogClass ui;
};

