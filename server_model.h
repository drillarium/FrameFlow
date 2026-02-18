#pragma once

#include <QUuid>

struct StreamingServer
{
  QUuid id;
  QString platform;
  QString name;
  QString url;
  QString key;
  bool enabled;
  int orderIndex;
};
