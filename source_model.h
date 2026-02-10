#pragma once

#include <QUuid>
#include <QJsonObject>
#include <QRect>

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
  EST_SCENE,
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

static QRect getSourceRect(const Source& _source)
{
  QRect rect;

  if(_source.config.contains("x") && _source.config.value("x").isDouble())
  {
    rect.setX(_source.config["x"].toInt());
  }
  if(_source.config.contains("y") && _source.config.value("y").isDouble())
  {
    rect.setY(_source.config["y"].toInt());
  }
  if(_source.config.contains("width") && _source.config.value("width").isDouble())
  {
    rect.setWidth(_source.config["width"].toInt());
  }
  if(_source.config.contains("height") && _source.config.value("height").isDouble())
  {
    rect.setHeight(_source.config["height"].toInt());
  }

  return rect;
}

static void setSourceRect(Source& _source, const QRect& _rect)
{
  _source.config["x"] = _rect.x();
  _source.config["y"] = _rect.y();
  _source.config["width"] = _rect.width();
  _source.config["height"] = _rect.height();
}
