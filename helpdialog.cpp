#include "helpdialog.h"
#include "version.h"
#include <QJsonDocument>
#include <QVersionNumber>
#include <QDir>
#include "confirmationdialog.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#endif // Q_OS_WIN

HelpDialog::HelpDialog(QWidget *parent)
:QDialog(parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  setFixedWidth(500);
  ui.currentVerLabel->setText(QString("Current v%1").arg(APP_VERSION));

  LicenseManager& lm = LicenseManager::instance();
  connect(&lm, &LicenseManager::statusChanged, this, &HelpDialog::onLicenseChanged);
  updateLicenseStatus();

  QObject::connect(&networkManager_, &QNetworkAccessManager::finished, this, &HelpDialog::onDownloadFinished);
  setUpdateStatus(E_CFOS_NONE);
}

HelpDialog::~HelpDialog()
{

}

void HelpDialog::onLicenseChanged(LicenseManager::Status newStatus)
{
  updateLicenseStatus();
}

void HelpDialog::updateLicenseStatus()
{
  LicenseManager& lm = LicenseManager::instance();
  LicenseManager::Status status = lm.status();
  QString id = lm.id();
  QDate expiration = lm.expiration();

  QString style;
  QString text;
  switch(status)
  {
    case LicenseManager::Status::Valid:            text = "Valid";             style = "color: #1B5E20; background: #E8F5E9; border: 1px solid #4CAF50; border-radius: 6px;"; break;
    case LicenseManager::Status::Trial:            text = "Trial";             style = "color: #8A6D00; background: #FFF8E1; border: 1px solid #FFB300; border-radius: 6px;"; break;
    case LicenseManager::Status::TrialExpired:     text = "Trial Expired";     style = "color: #B71C1C; background: #FDECEA; border: 1px solid #E53935; border-radius: 6px;"; break;
    case LicenseManager::Status::Expired:          text = "Expired";           style = "color: #B71C1C; background: #FDECEA; border: 1px solid #E53935; border-radius: 6px;"; break;
    case LicenseManager::Status::NotYetValid:      text = "Not Valid";         style = "color: #BF360C; background: #FFF3E0; border: 1px solid #FB8C00; border-radius: 6px;"; break;
    case LicenseManager::Status::WrongMachine:     text = "Wrong Machine";     style = "color: #B71C1C; background: #FDECEA; border: 1px solid #E53935; border-radius: 6px;"; break;
    case LicenseManager::Status::InvalidSignature: text = "Invalid Signature"; style = "color: #B71C1C; background: #FDECEA; border: 1px solid #E53935; border-radius: 6px;"; break;
    case LicenseManager::Status::InvalidLicense:   text = "Invalid License";   style = "color: #B71C1C; background: #FDECEA; border: 1px solid #E53935; border-radius: 6px;"; break;
    default:                                       text = "Unknown";           style = "color: #37474F; background: #ECEFF1; border: 1px solid #90A4AE; border-radius: 6px;"; break;
  }

  ui.licenseTypeLabel->setStyleSheet(style);
  ui.licenseTypeLabel->setText(text);

  ui.idLabel->setText(id);

  int daysToExpiration = QDate::currentDate().daysTo(expiration);
  ui.expirationDateLabel->setText(QString("%1 (%2d)").arg(expiration.toString(Qt::ISODate)).arg(daysToExpiration));
  if(daysToExpiration >= 0)
  {
    ui.expiresIconLabel->setPixmap(QPixmap(":/FrameFlow/check.svg"));
  }
  else
  {
    ui.expiresIconLabel->setPixmap(QPixmap(":/FrameFlow/closered.svg"));
  }

  ui.licenseLineEdit->setText(LicenseManager::currentMachineId());
}


void HelpDialog::onCheckForUpdates()
{
  if(updateStatus_ == E_CFOS_NONE)
  {
    setUpdateStatus(E_CFOS_DOWNLOADING_JSON);
  }
  else if(updateStatus_ == E_CFOS_READY_TO_DOWNLOAD)
  {
    setUpdateStatus(E_CFOS_DOWNLOADING_INSTALLER);
  }
  else if(updateStatus_ == E_CFOS_DOWNLOADED_INSTALLER)
  {
    QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Install updates", "Are you sure you want to exit application and install updates?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Yes);
    if(reply == QMessageBox::Yes)
    {
#ifdef Q_OS_WIN
      // Requesting elevation
      HINSTANCE result = ::ShellExecuteA(0, "runas", savePath_.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
#endif // Q_OS_WIN
      QCoreApplication::quit();
    }
  }
  else if(updateStatus_ == E_CFOS_ERROR)
  {
    setUpdateStatus(E_CFOS_DOWNLOADING_JSON);
  }
}

void HelpDialog::onDownloadFinished(QNetworkReply* _reply)
{
  if(!_reply) return;

  if(_reply->error() != QNetworkReply::NoError)
  {
    setUpdateStatus(E_CFOS_ERROR, _reply->errorString());
    _reply->deleteLater();
    networkReply_ = nullptr;
    return;
  }
  if(updateStatus_ == E_CFOS_DOWNLOADING_JSON)
  {
    QByteArray responseData = _reply->readAll();
    if(responseData.isEmpty())
    {
      setUpdateStatus(E_CFOS_ERROR, "No data");
      _reply->deleteLater();
      networkReply_ = nullptr;
      return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj = jsonDoc.object();
    QString latestVersion = jsonObj["version"].toString();
    QString downloadUrl = jsonObj["url"].toString();

    QVersionNumber currentVersion(APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
    QVersionNumber serverVersion = QVersionNumber::fromString(latestVersion);

    if(serverVersion > currentVersion)
    {
      installerUrl_ = downloadUrl;
      setUpdateStatus(E_CFOS_READY_TO_DOWNLOAD, QString("Version %1 available.").arg(latestVersion));
    }
    else
    {
      setUpdateStatus(E_CFOS_NONE, "You are running the latest version.");
    }
  }
  else if(updateStatus_ == E_CFOS_DOWNLOADING_INSTALLER)
  {
    if(saveFile_)
    {
      saveFile_->close();
      saveFile_->deleteLater();
      saveFile_ = nullptr;
    }

    setUpdateStatus(E_CFOS_DOWNLOADED_INSTALLER);
  }

  _reply->deleteLater();
  networkReply_ = nullptr;
}

void HelpDialog::setUpdateStatus(CheckForUpdatesStatus _status, const QString& _message)
{
  updateStatus_ = _status;

  if(networkReply_)
  {
    networkReply_->abort();
    networkReply_->deleteLater();
    networkReply_ = nullptr;
  }

  ui.messageLabel->setText(_message);
  ui.messageLabel->setStyleSheet(_status == E_CFOS_ERROR? "color: #B71C1C" : "color: white");
  
  if(_status == E_CFOS_NONE)
  {
    ui.progressBar->hide();
    ui.checkForUpdatesButton->setText("Check for updates");
    ui.checkForUpdatesButton->setEnabled(true);    
  }
  else if(_status == E_CFOS_DOWNLOADING_JSON)
  {
    ui.progressBar->hide();
    ui.checkForUpdatesButton->setText("Checking for updates ...");
    ui.checkForUpdatesButton->setEnabled(false);

    // async download JSON and parse it
    QString url = updateAddress_.url + "/" + updateAddress_.jsonFile;
    QUrl updateUrl(url);
    QNetworkRequest request(updateUrl);
    request.setTransferTimeout(3000);
    networkReply_ = networkManager_.get(request);
    connect(networkReply_, &QNetworkReply::sslErrors, [&](const QList<QSslError>& errors) {
      // WARNING: Ignoring SSL errors! Only for dev/testing
      qDebug() << "Ignoring SSL errors:" << errors;
      networkReply_->ignoreSslErrors();
    });
  }
  else if(_status == E_CFOS_ERROR)
  {
    ui.progressBar->hide();
    ui.checkForUpdatesButton->setText("Recheck for updates");
    ui.checkForUpdatesButton->setEnabled(true);
  }
  else if(_status == E_CFOS_READY_TO_DOWNLOAD)
  {
    ui.progressBar->hide();
    ui.checkForUpdatesButton->setText("Download");
    ui.checkForUpdatesButton->setEnabled(true);
  }
  else if(_status == E_CFOS_DOWNLOADING_INSTALLER)
  {
    ui.progressBar->setValue(0);
    ui.progressBar->show();
    ui.checkForUpdatesButton->setText("Downloading installer ...");
    ui.checkForUpdatesButton->setEnabled(false);

    if(saveFile_)
    {
      saveFile_->close();
      saveFile_->deleteLater();
      saveFile_ = nullptr;
    }
    savePath_ = QDir::tempPath() + "/installer.exe";
    saveFile_ = new QFile(savePath_);
    saveFile_->open(QIODevice::WriteOnly);

    // async download installer
    QUrl updateUrl(installerUrl_);
    QNetworkRequest request(updateUrl);
    request.setTransferTimeout(10000);
    networkReply_ = networkManager_.get(request);

    QObject::connect(networkReply_, &QNetworkReply::redirected, this, &HelpDialog::onRedirected);
    QObject::connect(networkReply_, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal) { ui.progressBar->setValue((bytesReceived * 100. / bytesTotal)); });
    QObject::connect(networkReply_, &QIODevice::readyRead, this, &HelpDialog::onReadyRead);
  }
  else if(_status == E_CFOS_DOWNLOADED_INSTALLER)
  {
    ui.progressBar->hide();
    ui.checkForUpdatesButton->setText("Install updates");
    ui.checkForUpdatesButton->setEnabled(true);
  }
}

void HelpDialog::onReadyRead()
{
  if(saveFile_)
  {
    saveFile_->write(networkReply_->readAll());
  }
}

void HelpDialog::onRedirected(const QUrl& _redirectUrl)
{
  if(!_redirectUrl.isEmpty())
  {
    networkReply_->deleteLater();

    QNetworkRequest newRequest(_redirectUrl);
    newRequest.setTransferTimeout(10000);
    networkReply_ = networkManager_.get(newRequest);

    // Reconnect signals for the new reply
    QObject::connect(networkReply_, &QNetworkReply::redirected, this, &HelpDialog::onRedirected);
    QObject::connect(networkReply_, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal) { ui.progressBar->setValue((bytesReceived * 100. / bytesTotal)); });
    QObject::connect(networkReply_, &QNetworkReply::readyRead, this, &HelpDialog::onReadyRead);
  }
}
