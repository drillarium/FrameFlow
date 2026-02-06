#include "sourcecolorbaserenderer.h"
#include "project_manager.h"

SourceColorBaseRenderer::SourceColorBaseRenderer()
:BaseRenderer()
{

}

SourceColorBaseRenderer::~SourceColorBaseRenderer()
{
  stop();
}


bool SourceColorBaseRenderer::start()
{
  stop();
  workerThread_ = std::thread([&] { running_ = true; workerThread(); });
  return true;
}

bool SourceColorBaseRenderer::stop()
{
  if(workerThread_.joinable())
  {
    running_ = false;
    workerThread_.join();
  }
  return true;
}

bool SourceColorBaseRenderer::setSource(Source _source)
{
  BaseRenderer::setSource(_source);

  if(_source.config.contains("color") && _source.config.value("color").isString())
  {
    QString color = _source.config.value("color").toString();

    // Expect format: #AARRGGBB
    if(color.size() == 9 && color.startsWith('#'))
    {
      QString aaHex = color.mid(1, 2);
      QString rrggbb = color.mid(3, 6);

      bool ok = false;
      int alphaDec = aaHex.toInt(&ok, 16);
      if(ok)
      {
        colorParams_ = QString("solid_color = '%1(%2)'").arg(rrggbb, QString::number(alphaDec)).toStdString();
      }
    }
  }
  
  return true;
}

void SourceColorBaseRenderer::workerThread()
{
  CComPtr<IMPreview> preview;
  CComBSTR channel;
  BOOL enableVideo = FALSE;
  BOOL enableAudio = FALSE;

  CComPtr<IMFFactory> factory;
  std::wstring wstr(colorParams_.begin(), colorParams_.end());
  CComBSTR colorParameters = CComBSTR(wstr.c_str());
  M_VID_PROPS vProps = { eMVF_HD1080_25p };
  M_AUD_PROPS aProps = { 2, 48000, 16, 0 };
  M_AV_PROPS avProps = { vProps, aProps };
  uint32_t samples = (aProps.nSamplesPerSec * 1) / 25;
  CComPtr<IMFFrame> blackFrame;

  // from current project
  ProjectManager &pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(project)
  {
    avProps.vidProps = getMVideoFormat(project->width, project->height, project->framerate);
    avProps.audProps = getMAudioProps();
    int fpsNum = 0, fpsDen = 0;
    getFactors(project->framerate, fpsNum, fpsDen);
    samples = (aProps.nSamplesPerSec * fpsDen) / fpsNum;
  }

  // coinit
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if(FAILED(hr)) goto cleanup;

  // preview
  hr = preview.CoCreateInstance(__uuidof(MFPreview));
  if(FAILED(hr)) goto cleanup;
  preview->PreviewEnable(channel, enableAudio, enableVideo);

  // factory
  hr = factory.CoCreateInstance(__uuidof(MFFactory));
  if(FAILED(hr)) goto cleanup;

  // blackframe
  hr = factory->MFFrameCreateFromMem(&avProps, 0, (long)samples, (LONGLONG)0, &blackFrame, colorParameters);
  if(FAILED(hr)) goto cleanup;

  while(running_)
  {
    // save last frame in ARGB format
    {
      std::lock_guard<std::mutex> lock(lastFrameMutex_);
      lastFrame_ = blackFrame;
    }

    // preview
    REFERENCE_TIME maxWait = -1;
    CComBSTR props;
    CComQIPtr<IMFReceiver> receiverPreview(preview);
    receiverPreview->ReceiverFramePut(blackFrame, maxWait, props);
  }

cleanup:
  {
    preview = NULL;
    blackFrame = NULL;
    factory = NULL;
  }
}

bool SourceColorBaseRenderer::getFrame(CComPtr<IMFFrame>& _frame)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) return false;

  lastFrame_->MFClone(&_frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);

  return true;
}
