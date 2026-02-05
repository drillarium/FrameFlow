#pragma once

#include <QUuid>
#include <QJsonObject>

enum class SourceType
{
  EST_COLOR,
  EST_FILE,
  EST_URL,
  EST_LIVE_SOURCE,
  EST_NDI,
  EST_DEVICE,
  EST_WEBCAM,
  EST_BROWSER,
  EST_SCREEN_CAPTURE,
  EST_TEXT,
  EST_LAST
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
