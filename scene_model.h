#pragma once

#include <QUuid>
#include <QJsonObject>
#include "source_model.h"

struct Scene {
  QUuid id;
  QUuid projectId;
  QString name;
  int orderIndex;
  QJsonObject settings; // resolution, fps, layout...
  QVector<Source> sources;
};
