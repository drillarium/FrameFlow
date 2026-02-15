#pragma once

#include "baserenderer.h"
#include <thread>
#include <mutex>

class SourceDeviceBaseRenderer : public BaseRenderer
{
public:
  SourceDeviceBaseRenderer(SourceType _type);
  ~SourceDeviceBaseRenderer();

  SourceType type() override { return type_; }
  bool start() override;
  bool stop() override;
  bool isRunning() override { return running_; }
  bool getFrame(CComPtr<IMFFrame>& _frame) override;
  bool setSource(Source _source);

  static QStringList listOfDevices(bool filter = true);
  static QStringList deviceProps(const QString &_device);

  static QStringList listOfNDILines();
  static QStringList NDIDeviceProps() { return deviceProps("NDI Receiver"); }

  static QStringList listOfMonitors();
  static QStringList listOfWindowHandlers();

protected:
  void workerThread();
  static QStringList listOfNDILines(const CComPtr<IMFDevice>& _device);
  static int monitorIndex(const QString &_name);
  static std::wstring windowHandlerAddress(const std::string &_window);

protected:
  SourceType type_;
  bool running_ = false;
  std::thread workerThread_;
  CComPtr<IMFFrame> lastFrame_;
  std::mutex lastFrameMutex_;
  std::string device_;
  std::string format_;
  std::string lineIn_;
  std::string monitor_;
  bool showMouse_ = true;
  std::string window_ = "<None>";
};
