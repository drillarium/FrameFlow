#pragma once

#include <QUuid>
#include <QJsonObject>

enum class SourceType
 {
  Camera,
  Rtsp,
  MediaFile,
  AudioInput,
  Plugin
};

struct Source
{
  QUuid id;
  QUuid sceneId;
  SourceType type;
  QString name;
  int orderIndex;
  QJsonObject config;   // persisted parameters
  QJsonObject state;    // optional, maybe not persisted
};
