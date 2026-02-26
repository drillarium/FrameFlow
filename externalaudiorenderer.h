#pragma once

#include <QObject>
#include <thread>
#include <mutex>
#include <MFormats.h>
#include <atlbase.h> // CComPtr

class ExternalAudioRenderer  : public QObject
{
Q_OBJECT

public:
  ExternalAudioRenderer(const QString &_name, QObject *parent = nullptr);
  ~ExternalAudioRenderer();

  QString name() { return deviceName_; }
  bool init();
  bool deinit();
  void update() { upddate_ = true; }
  bool vumeterValue(M_AUDIO_LOUDNESS &_al);
  void updateVolume(double _value);
  void mixAudio(CComPtr<IMFFrame> &_frame);

protected:
  void workerThread();

protected:
  QString deviceName_;
  bool running_ = false;
  std::thread workerThread_;
  bool upddate_ = true;
  CComPtr<IMFFrame> lastFrame_;
  std::mutex lastFrameMutex_;
  double volumeValue_ = 1.;
};

