#pragma once

#include <QUuid>
#include <QDateTime>

enum ENotificationSeverity { S_INFO, S_WARNING, S_ERROR };

struct Notification
{
  QUuid id;
  QString title;
  QString desciption;
  ENotificationSeverity severity;
  QDateTime creationTime;
  QDateTime modificationTime;
  bool read = false;
};
