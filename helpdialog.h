#pragma once

#include <QDialog>
#include "ui_helpdialog.h"
#include "license_manager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

// TODO: use public ip when available
struct CheckForUpdatesAddress
{
  QString url = "https://192.168.1.32:8443";
  QString jsonFile = "update.json";
};

enum CheckForUpdatesStatus
{
  E_CFOS_NONE,
  E_CFOS_ERROR,
  E_CFOS_DOWNLOADING_JSON,
  E_CFOS_READY_TO_DOWNLOAD,
  E_CFOS_DOWNLOADING_INSTALLER,
  E_CFOS_DOWNLOADED_INSTALLER,
};

class HelpDialog : public QDialog
{
Q_OBJECT

public:
  HelpDialog(QWidget *parent = nullptr);
  ~HelpDialog();

protected slots:
  void onLicenseChanged(LicenseManager::Status newStatus);
  void onCheckForUpdates();
  void onDownloadFinished(QNetworkReply* _reply);
  void onReadyRead();
  void onRedirected(const QUrl& _redirectUrl);

protected:
  void updateLicenseStatus();
  void setUpdateStatus(CheckForUpdatesStatus _status, const QString &_message = "");

private:
  Ui::HelpDialogClass ui;
  CheckForUpdatesStatus updateStatus_ = CheckForUpdatesStatus::E_CFOS_NONE;
  QNetworkAccessManager networkManager_;
  CheckForUpdatesAddress updateAddress_;
  QNetworkReply* networkReply_ = nullptr;
  QString installerUrl_;
  QFile* saveFile_ = nullptr;
  QString savePath_;
};

