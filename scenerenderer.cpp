#include "scenerenderer.h"
#include "project_manager.h"
#include "renderermanager.h"
#include <QElapsedTimer>

class Transitioner
{
public:
  void setScenes(const QUuid& _from, const QUuid& _to) { fromScene_ = _from; toScene_ = _to; }
  void setTransition(const Transition& _t) { transition_  = _t; }
  bool running() { return running_; }
  QUuid fromScene() { return fromScene_; }
  QUuid toScene() { return toScene_; }
  bool start()
  {
    if(running_) return false;

    timer_.start();
    running_ = true;

    return true;
  }
  bool checkDurationExceeded()
  {
    if(!running_) return true;

    int elapsed = timer_.elapsed();
    if(elapsed >= transition_.msDuration)
    {
      running_ = false;
    }

    return !running_;
  }
  bool exec(CComPtr<IMFFrame>& _fromFrame, const CComPtr<IMFFrame>& _toFrame, double _progress)
  {
    if(transition_.type == TransitionType::TT_CUT)
    {
      _fromFrame = _toFrame;
    }
    else if(transition_.type == TransitionType::TT_FADE)
    {
      CComPtr<IMFFrame> frameResult;
      CComBSTR ttype(L"fade");
      CComBSTR propList;
      CComBSTR converterId;
      _toFrame->MFTransition(_fromFrame, &frameResult, _progress, ttype, propList, converterId);
      _fromFrame = frameResult;
    }
    else if(transition_.type == TransitionType::TT_SLIDE)
    {
      CComPtr<IMFFrame> frameResult;
      CComBSTR ttype(L"slide");
      CComBSTR propList(L"slideStyle=SWAP bands=100");
      CComBSTR converterId;
      _toFrame->MFTransition(_fromFrame, &frameResult, _progress, ttype, propList, converterId);
      _fromFrame = frameResult;
    }

    return true;
  }

  // 0 - 1
  double progress()
  {
    if(!running_) return false;
    int elapsed = timer_.elapsed();
    if(elapsed >= transition_.msDuration) return 1;
    return (double) elapsed / transition_.msDuration;
  }

protected:
  QUuid fromScene_;
  QUuid toScene_;
  Transition transition_;
  bool running_ = false;
  bool inProgress_;
  QElapsedTimer timer_;
};

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

void SceneRenderer::renderScene(CComPtr<IMFFrame>& _frame, const QUuid& _scene)
{
  ProjectManager& pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(!project) return;

  // scene
  Scene scene;
  for(int i = 0; i < project->scenes.size(); i++)
  {
    if(project->scenes[i].id == _scene)
    {
      scene = project->scenes[i];
      break;
    }
  }

  // compose
  RendererManager& rm = RendererManager::instance();
  for(int i = 0; i < scene.sources.size(); i++)
  {
    CComPtr<IMFFrame> sourceFrame;
    QRect rect = getSourceRect(scene.sources[i]);
    rm.getFrame(scene.sources[i].id, sourceFrame);
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
        _frame->MFOverlay(sourceFrameResized, NULL, posX, posY, alpha, props, converter);
      }
    }
  }
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

  QUuid lastSceneUID;
  Transitioner transitioner;

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
    // update current project
    if(update_)
    {
      project = pm.currentProject();
      update_ = false;
    }

    // mode
    ProjectManager::WorkingMode workingMode = pm.workingMode();

    // current scene
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

    // transition?
    if(!lastSceneUID.isNull() && !sceneUID.isNull() && (sceneUID != lastSceneUID) && !transitioner.running())
    {
      if(project)
      {
        auto t = pm.currentTransition();
        if(t)
        {
          transitioner.setScenes(lastSceneUID, sceneUID);
          transitioner.setTransition(*t);
          transitioner.start();          
        }
      }
    }
    lastSceneUID = sceneUID;

    CComPtr<IMFFrame> frame;

    if(!transitioner.running())
    {
      blackFrame->MFClone(&frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);
      if(!frame) continue;

      renderScene(frame, sceneUID);
    }
    else
    {      
      blackFrame->MFClone(&frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);
      if(!frame) continue;

      CComPtr<IMFFrame> nextSceneFrame;
      blackFrame->MFClone(&nextSceneFrame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);
      if(!nextSceneFrame) continue;

      float progress = transitioner.progress(); // 0 - 1

      renderScene(frame, transitioner.fromScene());
      renderScene(nextSceneFrame, transitioner.toScene());
      transitioner.exec(frame, nextSceneFrame, progress);

      // check if completed
      transitioner.checkDurationExceeded();
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
    preview->PreviewEnable(channel, FALSE, FALSE);
    preview = NULL;
    blackFrame = NULL;
    factory = NULL;
  }
}

bool SceneRenderer::getFrame(CComPtr<IMFFrame>& _frame)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) return false;

  lastFrame_->MFClone(&_frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);

  return true;
}

