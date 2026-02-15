#include "license_manager.h"

#include <QFile>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QSettings>
#include <QSysInfo>
#include <QDir>

#include <openssl/evp.h>
#include <openssl/pem.h>

static const char* PUBLIC_KEY_PEM = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApj3FDXm/7z/7a92BboCt
Xsgl9Ej7173dhOZYZbVcw1NjGqdBy+LrWHO6WMMZDBpnozaw0lio6DYmBvArkPcB
qSYwKXctZCqToT9zhR+lG2JVXDyMHxYcl+5dC4nwRyJVlbxPZyTeXsK3cQkC2mER
23cTPiZcYntlvPBOKpxPxP5QlEjNmigqE0LGxtSvAHjO6ez0f9XKknCBNZgxUZ8Q
f82lZVPsmR0wMz2aRpzu0VHeiilUVtM2ekT6sIvfCRZPEUpzxS+YS6QCP+P4r2Be
t4xfBTYVCs90NlhZqoe3u2/JhKvaWoEO2xPN3GxYi3ldHygoN9QqwxxjLYI5o74f
gwIDAQAB
-----END PUBLIC KEY-----)";

LicenseManager& LicenseManager::instance()
{
  static LicenseManager inst;
  return inst;
}

LicenseManager::LicenseManager(QObject* parent)
:QObject(parent)
{
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(dataDir);
  m_licensePath = dir.filePath("license.json");

  storeFirstRunIfNeeded();

  // 10 minutes
  connect(&m_timer, &QTimer::timeout, this, &LicenseManager::revalidate);
  m_timer.setInterval(10 * 60 * 1000);

  revalidate();
  m_timer.start();
}

void LicenseManager::revalidate()
{
  loadLicense();

  Status newStatus;

  if(m_license.isEmpty())
  {
    newStatus = (daysSinceFirstRun() <= 14)? Status::Trial : Status::TrialExpired;
  }
  else
  {
    newStatus = validate(m_license);
  }

  setStatus(newStatus);

  if(newStatus == Status::Valid)
    setTier(m_license["tier"].toString("basic"));
  else
    setTier("basic");
}

void LicenseManager::setStatus(Status s)
{
  if(m_status == s) return;

  m_status = s;
  emit statusChanged(m_status);
}

void LicenseManager::setTier(const QString& t)
{
  if(m_tier == t) return;

  m_tier = t;
  emit tierChanged(m_tier);
}

LicenseManager::Status LicenseManager::status() const
{
  if(m_license.isEmpty()) {
    return daysSinceFirstRun() <= 14
      ? Status::Trial
      : Status::TrialExpired;
  }

  return validate(m_license);
}

QString LicenseManager::tier() const
{
  return m_license.value("tier").toString("basic");
}

QString LicenseManager::id() const
{
  return m_license.value("license_id").toString("");
}

QDate LicenseManager::expiration() const
{
  if(m_license.isEmpty())
  {
    QSettings s("AVIO", "FrameFlow");
    QDate first = s.value("first_run").toDate();
    return first.addDays(15);
  }
  return QDate::fromString(m_license["valid_until"].toString(), Qt::ISODate);
}

bool LicenseManager::loadLicense()
{
  QFile f(m_licensePath);
  if(!f.open(QIODevice::ReadOnly))
    return false;

  QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
  if(!doc.isObject())
    return false;

  m_license = doc.object();
  return true;
}

LicenseManager::Status LicenseManager::validate(const QJsonObject& lic) const
{
  if(lic.isEmpty())
    return Status::InvalidLicense;

  if(lic["machine_id"].toString() != currentMachineId())
    return Status::WrongMachine;

  QDate today = QDate::currentDate();
  QDate from = QDate::fromString(lic["valid_from"].toString(), Qt::ISODate);
  QDate until = QDate::fromString(lic["valid_until"].toString(), Qt::ISODate);

  if(today < from)
    return Status::NotYetValid;

  if(today > until)
    return Status::Expired;

  QByteArray payload = licensePayload(lic).toUtf8();
  QByteArray sig = lic["signature"].toString().toUtf8();

  if(!verifySignature(payload, sig))
    return Status::InvalidSignature;

  return Status::Valid;
}

QString LicenseManager::licensePayload(const QJsonObject& o) const
{
  return QString("%1|%2|%3|%4|%5")
    .arg(o["license_id"].toString())
    .arg(o["tier"].toString())
    .arg(o["machine_id"].toString())
    .arg(o["valid_from"].toString())
    .arg(o["valid_until"].toString());
}

bool LicenseManager::verifySignature(const QByteArray& data,
  const QByteArray& signatureBase64) const
{
  BIO* bio = BIO_new_mem_buf(PUBLIC_KEY_PEM, -1);
  EVP_PKEY* pubKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
  BIO_free(bio);

  if(!pubKey)
    return false;

  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pubKey);
  EVP_DigestVerifyUpdate(ctx, data.data(), data.size());

  QByteArray sig = QByteArray::fromBase64(signatureBase64);

  bool ok = EVP_DigestVerifyFinal(
    ctx,
    reinterpret_cast<const unsigned char*>(sig.data()),
    sig.size()) == 1;

  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(pubKey);
  return ok;
}

QString LicenseManager::currentMachineId()
{
  QString raw;

#ifdef Q_OS_WIN
  QSettings reg("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography", QSettings::NativeFormat);
  raw = reg.value("MachineGuid").toString();
#elif defined(Q_OS_MAC)
  raw = QSysInfo::machineUniqueId();
#elif defined(Q_OS_LINUX)
  QFile f("/etc/machine-id");
  if(f.open(QIODevice::ReadOnly))
    raw = QString::fromUtf8(f.readAll()).trimmed();
#endif

  return QCryptographicHash::hash(
    raw.toUtf8(),
    QCryptographicHash::Sha256
  ).toHex();
}

void LicenseManager::storeFirstRunIfNeeded() const
{
  QSettings s("AVIO", "FrameFlow");
  if(!s.contains("first_run"))
  {
    s.setValue("first_run", QDate::currentDate());
  }
}

int LicenseManager::daysSinceFirstRun() const
{
  QSettings s("AVIO", "FrameFlow");
  QDate first = s.value("first_run").toDate();
  return first.daysTo(QDate::currentDate());
}
