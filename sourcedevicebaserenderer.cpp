#include "sourcedevicebaserenderer.h"
#include "project_manager.h"
#include <chrono>
#include <QScreen>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

using namespace std::chrono_literals;

QStringList SourceDeviceBaseRenderer::listOfDevices(bool filter)
{
  QStringList l;

  CComPtr<IMFDevice> device;
  device.CoCreateInstance(__uuidof(MFLive));

  int deviceCount = 0;
  device->DeviceGetCount(eMFDT_Video, &deviceCount);
  for(int i = 0; i < deviceCount; i++)
  {
    CComBSTR name;
    BOOL busy = FALSE;
    device->DeviceGetByIndex(eMFDT_Video, i, &name, &busy);
    if(name)
    {
      QString str = QString::fromWCharArray(name);
      if(!filter || (!str.contains("Medialooks") && str != "<None>" && str != "NDI Receiver"))
      {
        l.push_back(str);
      }
    }
  }

  device->DeviceClose();

  return l;
}

QStringList SourceDeviceBaseRenderer::deviceProps(const QString& _device)
{
  QStringList l;

  CComPtr<IMFDevice> device;
  device.CoCreateInstance(__uuidof(MFLive));

  int deviceIndex = -1;
  int deviceCount = 0;
  device->DeviceGetCount(eMFDT_Video, &deviceCount);
  for(int i = 0; i < deviceCount; i++)
  {
    CComBSTR name;
    BOOL busy = FALSE;
    device->DeviceGetByIndex(eMFDT_Video, i, &name, &busy);
    if(name)
    {
      QString str = QString::fromWCharArray(name);
      if(_device == str)
      {
        deviceIndex = i;
        break;
      }
    }
  }

  CComBSTR params;
  device->DeviceSet(eMFDT_Video, deviceIndex, params);

  CComQIPtr<IMFFormat> format(device);
  int formatsCount = 0;
  format->FormatVideoGetCount(eMFormatType::eMFT_Input, &formatsCount);

  CComBSTR name;
  for(int i = 0; i < formatsCount; i++)
  {
    M_VID_PROPS props = {};
    format->FormatVideoGetByIndex(eMFormatType::eMFT_Input, i, &props, &name);
    if(name)
    {
      QString str = QString::fromWCharArray(name);
      l.push_back(str);
    }
  }

  device->DeviceClose();

  return l;
}

QStringList SourceDeviceBaseRenderer::listOfNDILines()
{
  CComPtr<IMFDevice> device;
  device.CoCreateInstance(__uuidof(MFLive));

  int deviceIndex = -1;
  int deviceCount = 0;
  device->DeviceGetCount(eMFDT_Video, &deviceCount);
  for(int i = 0; i < deviceCount; i++)
  {
    CComBSTR name;
    BOOL busy = FALSE;
    device->DeviceGetByIndex(eMFDT_Video, i, &name, &busy);
    if(name)
    {
      QString str = QString::fromWCharArray(name);
      if(str == "NDI Receiver")
      {
        deviceIndex = i;
        break;
      }
    }
  }

  CComBSTR params;
  device->DeviceSet(eMFDT_Video, deviceIndex, params);

  QStringList ret = listOfNDILines(device);

  device->DeviceClose();
  
  return ret;
}

QStringList SourceDeviceBaseRenderer::listOfNDILines(const CComPtr<IMFDevice>& _device)
{
  QStringList l;

  CComQIPtr<IMFProps> props(_device);
  CComBSTR prop(L"line-in");

  bool retrying = false;
  while(1)
  {
    int linesCount = 0;
    props->PropsOptionGetCount(prop, &linesCount);

    CComBSTR option, help;
    for(int i = 0; i < linesCount; i++)
    {
      props->PropsOptionGetByIndex(prop, i, &option, &help);
      if(help)
      {
        QString str = QString::fromWCharArray(help);
        l.push_back(str);
      }
    }
    if(linesCount > 0 || retrying) break;
    retrying = true;
    std::this_thread::sleep_for(1s);
  }

  return l;
}

M_VID_PROPS VID_PROP(CComQIPtr<IMFFormat> &_format, QString &_prop)
{
  int formatsCount = 0;
  _format->FormatVideoGetCount(eMFormatType::eMFT_Input, &formatsCount);

  CComBSTR name;
  for(int i = 0; i < formatsCount; i++)
  {
    M_VID_PROPS props = {};
    _format->FormatVideoGetByIndex(eMFormatType::eMFT_Input, i, &props, &name);
    if(name)
    {
      QString str = QString::fromWCharArray(name);
      if(str == _prop)
      {
        return props;
      }
    }
  }

  return M_VID_PROPS();
}

QStringList SourceDeviceBaseRenderer::listOfMonitors()
{
  QStringList monitors;
  const QList<QScreen*> screens = QGuiApplication::screens();
  for(QScreen* screen : screens)
  {
    monitors << screen->name();
  }
  return monitors;
}

int SourceDeviceBaseRenderer::monitorIndex(const QString& _name)
{
  QStringList l = SourceDeviceBaseRenderer::listOfMonitors();
  return l.indexOf(_name);
}

QMap<QString, QString> windowHandlers()
{
  QMap<QString, QString> ret;

  EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
    {
      auto mmap = reinterpret_cast<QMap<QString, QString>*>(lParam);

      // 1️ Must be visible
      if(!IsWindowVisible(hwnd))
        return TRUE;

      // 2️ Must not have owner (no parent/main only)
      if(GetWindow(hwnd, GW_OWNER) != nullptr)
        return TRUE;

      // 3️ Skip tool windows
      LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
      if(exStyle & WS_EX_TOOLWINDOW)
        return TRUE;

      // 4️ Skip cloaked windows (Windows 8+ UWP background)
      BOOL cloaked = FALSE;
      if(SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED,
        &cloaked, sizeof(cloaked))) && cloaked)
        return TRUE;

      // 5️ Must have title
      int length = GetWindowTextLength(hwnd);
      if(length == 0)
        return TRUE;

      int l = GetWindowTextLength(hwnd);
      std::wstring title(l, L'\0');
      GetWindowText(hwnd, &title[0], l + 1);

      QString adr = "0x" + QString::number((unsigned long long) hwnd, 16);
      mmap->insert(adr, QString::fromStdWString(title));

      return TRUE;

    }, reinterpret_cast<LPARAM>(&ret));

  return ret;
}


QStringList SourceDeviceBaseRenderer::listOfWindowHandlers()
{
  QMap<QString, QString> h = windowHandlers();
  return h.values();
}

std::wstring SourceDeviceBaseRenderer::windowHandlerAddress(const std::string& _window)
{
  QMap<QString, QString> h = windowHandlers();
  QString w = QString::fromStdString(_window);
  for(auto it = h.cbegin(); it != h.cend(); ++it)
  {
    if(it.value() == w)
    {
      return it.key().toStdWString();
    }
  }

  return std::wstring();
}

SourceDeviceBaseRenderer::SourceDeviceBaseRenderer(SourceType _type)
:BaseRenderer()
,type_(_type)
{

}

SourceDeviceBaseRenderer::~SourceDeviceBaseRenderer()
{
  stop();
}

bool SourceDeviceBaseRenderer::start()
{
  stop();
  workerThread_ = std::thread([&] { running_ = true; workerThread(); });
  return true;
}

bool SourceDeviceBaseRenderer::stop()
{
  if(workerThread_.joinable())
  {
    running_ = false;
    workerThread_.join();
  }
  return true;
}

bool SourceDeviceBaseRenderer::getFrame(CComPtr<IMFFrame>& _frame)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) return false;

  lastFrame_->MFClone(&_frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);

  return true;
}

bool SourceDeviceBaseRenderer::setSource(Source _source)
{
  BaseRenderer::setSource(_source);

  if(_source.config.contains("window") && _source.config.value("window").isString())
  {
    window_ = _source.config.value("window").toString().toStdString();
  }

  if(_source.config.contains("device") && _source.config.value("device").isString())
  {
    device_ = _source.config.value("device").toString().toStdString();
  }
  else if(_source.type == SourceType::EST_NDI)
  {
    device_ = "NDI Receiver";
  }
  else if(_source.type == SourceType::EST_SCREEN_CAPTURE)
  {
    device_ = "Medialooks DXGI/DX11 ScreenCapture";
    if(window_ != "<None>")
    {
      device_ = "Medialooks Windows Graphics Capture";
    }
  }

  if(_source.config.contains("format") && _source.config.value("format").isString())
  {
    format_ = _source.config.value("format").toString().toStdString();
  }

  if(_source.config.contains("line") && _source.config.value("line").isString())
  {
    lineIn_ = _source.config.value("line").toString().toStdString();
  }

  if(_source.config.contains("monitor") && _source.config.value("monitor").isString())
  {
    monitor_ = _source.config.value("monitor").toString().toStdString();
  }

  if(_source.config.contains("show_mouse") && _source.config.value("show_mouse").isBool())
  {
    showMouse_ = _source.config.value("show_mouse").toBool();
  }

  return true;
}

void SourceDeviceBaseRenderer::workerThread()
{
  CComPtr<IMPreview> preview;
  CComBSTR channel;
  BOOL enableVideo = FALSE;
  BOOL enableAudio = FALSE;

  M_VID_PROPS vProps = { eMVF_HD1080_25p };
  M_AUD_PROPS aProps = { 2, 48000, 16, 0 };
  M_AV_PROPS avProps = { vProps, aProps };

  CComPtr<IMFDevice> device;
  int frameNumber = -1;
  CComBSTR deviceParams;
  QStringList dl = SourceDeviceBaseRenderer::listOfDevices(false);
  int deviceIndex = dl.indexOf(QString::fromStdString(device_));
  bool lineFound = false;

  // from current project
  ProjectManager& pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(project)
  {
    avProps.vidProps = getMVideoFormat(project->width, project->height, project->framerate);
    avProps.audProps = getMAudioProps();
  }

  // coinit
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if(FAILED(hr)) goto cleanup;

  // preview
  hr = preview.CoCreateInstance(__uuidof(MFPreview));
  if(FAILED(hr)) goto cleanup;
  preview->PreviewEnable(channel, enableAudio, enableVideo);

  // device
  hr = device.CoCreateInstance(__uuidof(MFLive));
  if(FAILED(hr)) goto cleanup; 

  hr = device->DeviceSet(eMFDT_Video, deviceIndex, deviceParams);
  if(FAILED(hr)) goto cleanup;

  if(format_.compare("<Auto/Not Specified>"))
  {
    CComQIPtr<IMFFormat> format(device);
    QString f = QString::fromStdString(format_);
    M_VID_PROPS props = VID_PROP(format, f);
    format->FormatVideoSet(eMFT_Input, &props);
  }

  if(monitor_.length() > 0)
  {
    int monitorIndex = SourceDeviceBaseRenderer::monitorIndex(QString::fromStdString(monitor_));
    if(monitorIndex < 0) monitorIndex = 0;

    CComQIPtr<IMFProps> props(device);
    CComBSTR prop(L"capture.screen_index");
    std::wstring ws = std::to_wstring(monitorIndex);
    CComBSTR value(ws.c_str());
    props->PropsSet(prop, value);
  
    prop = CComBSTR(L"capture.show_mouse");
    value = showMouse_? CComBSTR(L"show") : CComBSTR(L"hide");
    props->PropsSet(prop, value);

    if(window_ != "<None>")
    {
      prop = CComBSTR(L"capture.follow_window_by_handle");
      std::wstring ws = SourceDeviceBaseRenderer::windowHandlerAddress(window_);
      CComBSTR value(ws.c_str());
      props->PropsSet(prop, value);
    }
  }

  while(running_)
  {
    CComPtr<IMFFrame> frame;

    if(lineIn_.size() > 0 && !lineFound)
    {
      QStringList dl = listOfNDILines(device);
      int deviceLineIndex = dl.indexOf(lineIn_);
      lineFound = (deviceLineIndex >= 0);
      if(lineFound)
      {
        CComQIPtr<IMFProps> props(device);
        CComBSTR prop(L"line-in");
        props->PropsOptionSetByIndex(prop, deviceLineIndex);
      }
    }

    // read
    CComQIPtr<IMFSource> source(device);
    REFERENCE_TIME maxWait = -1;
    CComBSTR hints;
    hr = source->SourceFrameConvertedGetByNumber(&avProps, frameNumber, maxWait, &frame, hints);     

    if(frame)
    {
      // loop
      M_TIME timecode;
      frame->MFTimeGet(&timecode);

      // save last frame in ARGB format
      {
        std::lock_guard<std::mutex> lock(lastFrameMutex_);
        lastFrame_ = frame;
      }

      // preview
      REFERENCE_TIME maxWait = -1;
      CComBSTR props;
      CComQIPtr<IMFReceiver> receiverPreview(preview);
      receiverPreview->ReceiverFramePut(frame, maxWait, props);
    }
    else
    {
      std::this_thread::sleep_for(1s);
    }
  }

cleanup:
  {
    preview = NULL;
    if(device)
    {
      device->DeviceClose();
      device = NULL;
    }
  }
}

