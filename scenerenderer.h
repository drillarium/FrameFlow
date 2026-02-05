#pragma once

#include "baserenderer.h"

#include <QObject>
#include <thread>
#include <MFormats.h>
#include <atlbase.h> // CComPtr
#include <QImage>

class SceneRenderer : public QObject
{
Q_OBJECT

public:
  SceneRenderer(QObject *parent = nullptr);
  ~SceneRenderer();

  bool start();
  bool stop();

protected:
  void workerThread();

signals:
  void onNewImage(const QImage image);

protected:
  bool running_ = false;
  std::thread workerThread_;
  CComPtr<IMPreview> preview_;
};
