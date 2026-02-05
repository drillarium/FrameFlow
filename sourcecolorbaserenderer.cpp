#include "sourcecolorbaserenderer.h"
#include "project_manager.h"

SourceColorBaseRenderer::SourceColorBaseRenderer()
:SourceRenderer()
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
