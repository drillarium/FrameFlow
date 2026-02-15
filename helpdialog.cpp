#include "helpdialog.h"
#include "version.h"

HelpDialog::HelpDialog(QWidget *parent)
:QDialog(parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  ui.currentVerLabel->setText(QString("Current v%1").arg(APP_VERSION_STR));

  LicenseManager& lm = LicenseManager::instance();
  connect(&lm, &LicenseManager::statusChanged, this, &HelpDialog::onLicenseChanged);
  updateLicenseStatus();
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
    case LicenseManager::Status::Valid:            text = "Valid";             style = "color: white; background: #1FA831; border: 1px solid white; border-radius: 6px;"; break;
    case LicenseManager::Status::Trial:            text = "Trial";             style = "color: #ECC115; background: #3E3618; border: 1px solid #ECC115; border-radius: 6px;"; break;
    case LicenseManager::Status::TrialExpired:     text = "Trial Expired";     style = "color: white; background: #DF4E12; border: 1px solid white; border-radius: 6px;"; break;
    case LicenseManager::Status::Expired:          text = "Expired";           style = "color: white; background: #DF4E12; border: 1px solid white; border-radius: 6px;"; break;
    case LicenseManager::Status::NotYetValid:      text = "Not Valid";         style = "color: white; background: #DF4E12; border: 1px solid white; border-radius: 6px;"; break;
    case LicenseManager::Status::WrongMachine:     text = "Wrong Machine";     style = "color: white; background: #DF4E12; border: 1px solid white; border-radius: 6px;"; break;
    case LicenseManager::Status::InvalidSignature: text = "Invalid Signature"; style = "color: white; background: #DF4E12; border: 1px solid white; border-radius: 6px;"; break;
    case LicenseManager::Status::InvalidLicense:   text = "Invalid License";   style = "color: white; background: #DF4E12; border: 1px solid white; border-radius: 6px;"; break;
    default:                                       text = "Unknown";           style = "color: white; background: #DF4E12; border: 1px solid white; border-radius: 6px;"; break;
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
