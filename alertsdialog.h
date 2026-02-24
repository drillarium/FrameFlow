#pragma once

#include <QDialog>
#include "ui_alertsdialog.h"
#include "notification_model.h"

class AlertsDialog : public QDialog
{
Q_OBJECT

public:
  AlertsDialog(QWidget *parent = nullptr);
  ~AlertsDialog();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void checkNumNotifications();

protected slots:
  void onClearAll();
  void onMarkAll();
  void onNotificationsSelectionChange();
  void updateNotifications();

private:
  Ui::AlertsDialogClass ui;
  bool clear_ = false;
};

