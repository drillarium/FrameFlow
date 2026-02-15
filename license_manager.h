#pragma once

#include <QObject>
#include <QTimer>
#include <QJsonObject>

class LicenseManager : public QObject
{
  Q_OBJECT

public:
  enum class Status {
    Valid,
    Trial,
    TrialExpired,
    Expired,
    NotYetValid,
    WrongMachine,
    InvalidSignature,
    InvalidLicense
  };
  Q_ENUM(Status)

  static LicenseManager& instance();

  Status status() const;
  QString tier() const;
  QString id() const;
  QDate expiration() const;

  static QString currentMachineId();

signals:
  void statusChanged(LicenseManager::Status newStatus);
  void tierChanged(const QString& newTier);

private slots:
  void revalidate();

private:
  explicit LicenseManager(QObject* parent = nullptr);
  void setStatus(Status s);
  void setTier(const QString& t);

  bool loadLicense();
  Status validate(const QJsonObject& lic) const;

  bool verifySignature(const QByteArray& payload, const QByteArray& signatureBase64) const;

  QString licensePayload(const QJsonObject& o) const;

  int daysSinceFirstRun() const;
  void storeFirstRunIfNeeded() const;

private:
  QString m_licensePath;
  QJsonObject m_license;

  Status m_status = Status::InvalidLicense;
  QString m_tier = "basic";
  QTimer m_timer;
};
