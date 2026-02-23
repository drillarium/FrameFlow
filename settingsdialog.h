#pragma once

#include <QDialog>
#include "ui_settingsdialog.h"
#include "server_model.h"
#include "encoding_settings_model.h"

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  SettingsDialog(QWidget *_parent = nullptr);
  ~SettingsDialog();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void addStreamingServer(const StreamingServer& _ss);
  void loadEncoderSettings(const EncoderSettings &_settings);

protected slots:
  void onAddStreamServer();
  void onSaveEncoding();
  void onSelectOutputFolder();
  void onVideoBitrateChange();

signals:
  void saveEncoding(const EncoderSettings &_settings);

private:
  Ui::SettingsDialogClass ui;
};

