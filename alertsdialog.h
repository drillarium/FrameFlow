#pragma once

#include <QDialog>
#include "ui_alertsdialog.h"

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

private:
  Ui::AlertsDialogClass ui;
};

