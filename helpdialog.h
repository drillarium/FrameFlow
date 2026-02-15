#pragma once

#include <QDialog>
#include "ui_helpdialog.h"
#include "license_manager.h"

class HelpDialog : public QDialog
{
Q_OBJECT

public:
  HelpDialog(QWidget *parent = nullptr);
  ~HelpDialog();

protected slots:
  void onLicenseChanged(LicenseManager::Status newStatus);

protected:
  void updateLicenseStatus();

private:
  Ui::HelpDialogClass ui;
};

