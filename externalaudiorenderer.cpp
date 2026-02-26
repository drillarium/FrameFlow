#include "externalaudiorenderer.h"
#include <chrono>
#include "project_manager.h"
#include "baserenderer.h"

using namespace std::chrono_literals;

ExternalAudioRenderer::ExternalAudioRenderer(const QString& _name, QObject *parent)
:QObject(parent)
,deviceName_(_name)
{

}

ExternalAudioRenderer::~ExternalAudioRenderer()
{

}

bool ExternalAudioRenderer::init()
{
  workerThread_ = std::thread([&] { running_ = true; workerThread(); });
  return true;
}

bool ExternalAudioRenderer::deinit()
{
  if(workerThread_.joinable())
  {
    running_ = false;
    workerThread_.join();
  }
  return true;
}

double volume_to_audiogain(double _volume)
{
  if(_volume <= 0.000001) return -100.0;

  return 20.0 * std::log10(_volume);
}

void ExternalAudioRenderer::workerThread()
{
  CComPtr<IMFDevice> device;
  HRESULT hr = device.CoCreateInstance(__uuidof(MFLive));

  int index = -1;

  // index?  
  int count = 0;
  device->DeviceGetCount(eMFDeviceType::eMFDT_ExtAudio, &count);
  for(int i = 0; i < count; i++)
  {
    CComBSTR name;
    BOOL busy = FALSE;
    device->DeviceGetByIndex(eMFDeviceType::eMFDT_ExtAudio, i, &name, &busy);
    QString qname = QString::fromWCharArray(name, name.Length());
    if(qname == deviceName_)
    {
      index = i;
      break;
    }
  }

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
      if(str == "<None>")
      {
        deviceIndex = i;
        break;
      }
    }
  }

  CComBSTR params;
  device->DeviceSet(eMFDeviceType::eMFDT_Video, deviceIndex, params);
  device->DeviceSet(eMFDeviceType::eMFDT_ExtAudio, index, params);

  CComPtr<IMPreview> preview;
  CComBSTR channel;
  BOOL enableVideo = FALSE;
  BOOL enableAudio = FALSE;
  hr = preview.CoCreateInstance(__uuidof(MFPreview));
  preview->PreviewEnable(channel, enableAudio, enableVideo);

  M_VID_PROPS vProps = { eMVF_HD1080_25p };
  M_AUD_PROPS aProps = { 2, 48000, 16, 0 };
  M_AV_PROPS avProps = { vProps, aProps };

  while(running_)
  {
    if(upddate_)
    {
      upddate_ = false;

      // from current project
      ProjectManager& pm = ProjectManager::instance();
      auto project = pm.currentProject();
      if(project)
      {
        avProps.vidProps = BaseRenderer::getMVideoFormat(project->width, project->height, project->framerate);
        avProps.audProps = BaseRenderer::getMAudioProps();
      }

      // change format... from SampleFileWriter... not working
      CComQIPtr<IMFFormat> format(device);
      hr = format->FormatVideoSet(eMFT_Input, &avProps.vidProps);
    }

    CComPtr<IMFFrame> frame;

    CComQIPtr<IMFSource> source(device);
    REFERENCE_TIME maxWait = -1;
    CComBSTR hints;
    hr = source->SourceFrameConvertedGet(&avProps, maxWait, &frame, hints);
    // hr = source->SourceFrameGet(maxWait, &frame, hints);
    if(frame)
    {
      M_AV_PROPS avProps = {};
      frame->MFAVPropsGet(&avProps, NULL);

      // apply volume
      double audioGain = volume_to_audiogain(volumeValue_ * volumeValue_);
      CComBSTR channelList(L"recalc_vu_meters");
      frame->MFAudioGain(channelList, audioGain, audioGain);

      // save
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

  device->DeviceClose();
}

bool ExternalAudioRenderer::vumeterValue(M_AUDIO_LOUDNESS &_al)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) return false;

  M_AV_PROPS avPRops = {};
  lastFrame_->MFAVPropsGet(&avPRops, NULL);
  _al = avPRops.ancData.audOutput;
  return true;
}

void ExternalAudioRenderer::updateVolume(double _value)
{
  volumeValue_ = _value;
}

void ExternalAudioRenderer::mixAudio(CComPtr<IMFFrame>& _frame)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) { return; }

  CComPtr<IMFFrame> clonedFrame;
  lastFrame_->MFClone(&clonedFrame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_ARGB32);
  LONG size = 0;
  LONGLONG data = 0;
  clonedFrame->MFVideoGetBytes(&size, &data);
  memset((void *) data, 0, size);

  M_AV_PROPS avProps = {};
  LONG audioSamples = 0;
  clonedFrame->MFAVPropsGet(&avProps, &audioSamples);

  clonedFrame->MFAudioGetBytes(&size, &data);

  int xPos = 0;
  int yPos = 0;
  double alpha = 1;
  CComBSTR props;
  CComBSTR audioBuffer;
  _frame->MFOverlay(clonedFrame, NULL, xPos, yPos, alpha, props, audioBuffer);
}