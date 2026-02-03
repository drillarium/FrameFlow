#pragma once

#include <QObject>

enum ESourceType
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

class SourceManager  : public QObject
{
Q_OBJECT

public:
  static SourceManager& instance();

private:
  explicit SourceManager(QObject* parent = nullptr);
  ~SourceManager() = default;

  Q_DISABLE_COPY_MOVE(SourceManager)
};
