#include "scenerenderer.h"
#include "project_manager.h"
#include "renderermanager.h"

SceneRenderer::SceneRenderer(EPreviewMode _mode)
:BaseRenderer()
,mode_(_mode)
{

}

SceneRenderer::~SceneRenderer()
{
  stop();
}

bool SceneRenderer::start()
{
  stop();
  workerThread_ = std::thread([&] { running_ = true; workerThread(); });
  return true;
}

bool SceneRenderer::stop()
{
  if(workerThread_.joinable())
  {
    running_ = false;
    workerThread_.join();
  }
  return true;
}

void SceneRenderer::workerThread()
{
  CComPtr<IMPreview> preview;
  CComBSTR channel;
  BOOL enableVideo = FALSE;
  BOOL enableAudio = FALSE;

  CComPtr<IMFFactory> factory;
  CComBSTR colorParameters = L"solid_color='Black(255)'";
  M_VID_PROPS vProps = { eMVF_HD1080_25p };
  M_AUD_PROPS aProps = { 2, 48000, 16, 0 };
  M_AV_PROPS avProps = { vProps, aProps };
  uint32_t samples = (aProps.nSamplesPerSec * 1) / 25;
  CComPtr<IMFFrame> blackFrame;

  // from current project
  ProjectManager& pm = ProjectManager::instance();
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
  hr = factory->MFFrameCreateFromMem(&avProps, 0, (long) samples, (LONGLONG) 0, &blackFrame, colorParameters);
  if(FAILED(hr)) goto cleanup;

  while(running_)
  {
    ProjectManager::WorkingMode workingMode = pm.workingMode();

    // current scene
    Scene scene;
    QUuid sceneUID;
    if(mode_ == EPreviewMode::PM_PROGRAM)
    {
      if(workingMode == ProjectManager::CONT)
      {
        sceneUID = pm.currentSceneId();
      }
      else if(workingMode == ProjectManager::STUDIO)
      {
        sceneUID = pm.currentStudioSceneId();
      }
    }
    else if(mode_ == EPreviewMode::PM_PREVIEW)
    {
      if(workingMode == ProjectManager::CONT)
      {
        // NOOP
      }
      else if(workingMode == ProjectManager::STUDIO)
      {
        sceneUID = pm.currentSceneId();
      }
    }
    if(project)
    {
      for(int i = 0; i < project->scenes.size(); i++)
      {
        if(project->scenes[i].id == sceneUID)
        {
          scene = project->scenes[i];
          break;
        }
      }
    }

    CComPtr<IMFFrame> frame;
    blackFrame->MFClone(&frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);
    if(!frame) continue;

    // compose
    RendererManager& rm = RendererManager::instance();
    for(int i = 0; i < scene.sources.size(); i++)
    {
      CComPtr<IMFFrame> sourceFrame;
      QRect rect;     
      rm.getFrame(scene.sources[i].id, sourceFrame, rect);
      if(sourceFrame)
      {
        int resizeField = -1;
        CComPtr<IMFFrame> sourceFrameResized;
        CComBSTR props;
        CComBSTR converter;
        sourceFrame->MFResize(eMFCC::eMFCC_Default, rect.width(), rect.height(), resizeField, &sourceFrameResized, props, converter);
        if(sourceFrameResized)
        {
          int posX = rect.x();
          int posY = rect.y();
          double alpha = 1.;
          CComBSTR props;
          CComBSTR converter;
          frame->MFOverlay(sourceFrameResized, NULL, posX, posY, alpha, props, converter);
        }
      }
    }

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

cleanup:
  {
    preview = NULL;
    blackFrame = NULL;
    factory = NULL;
  }
}

bool SceneRenderer::getFrame(CComPtr<IMFFrame>& _frame, QRect& _rect)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) return false;

  lastFrame_->MFClone(&_frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);

  return true;
}

bool SceneRenderer::getFrame(CComPtr<IMFFrame>& _frame)
{
  QRect r;
  return getFrame(_frame, r);
}
