#pragma once

#include <QUuid>
#include <QDateTime>
#include "scene_model.h"

struct Project
{
  QUuid id;
  QString name;
  QString description;
  int schemaVersion;
  int width = 1920;
  int height = 1080;
  double framerate = 30.0;
  QDateTime createdAt;
  QDateTime modifiedAt;
  QVector<Scene> scenes;
};

