#include "renderermanager.h"
#include <QTimer>
#include "project_manager.h"
#include "baserenderer.h"

RendererManager& RendererManager::instance()
{
  static RendererManager instance;
  return instance;
}

RendererManager::RendererManager(QObject *parent)
:QObject(parent)
,previewRenderer_(EPreviewMode::PM_PREVIEW)
,programRenderer_(EPreviewMode::PM_PROGRAM)
{
  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &RendererManager::onTimeout);
  timer->start(100); // 100 ms

  connect(&programRenderer_, &SceneRenderer::onStartStreaming, this, &RendererManager::onStartStreaming);
  connect(&programRenderer_, &SceneRenderer::onStopStreaming, this, &RendererManager::onStopStreaming);
  connect(&programRenderer_, &SceneRenderer::onStartRecording, this, &RendererManager::onStartRecording);
  connect(&programRenderer_, &SceneRenderer::onStopRecording, this, &RendererManager::onStopRecording);
}

QImage convertFrame(CComPtr<IMFFrame>& _frame)
{
  if(!_frame) return QImage();

  CComPtr<IMFFrame> ARGBFrame;
  _frame->MFClone(&ARGBFrame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_ARGB32);
  if(!ARGBFrame) return QImage();

  M_AV_PROPS avProps = {};
  ARGBFrame->MFAVPropsGet(&avProps, NULL);
  MF_VID_PTR vPtr;
  ARGBFrame->MFVideoGetBytesEx(&vPtr);

  const uint8_t *data =  (const uint8_t *) vPtr.lpVideoPlanes[0];
  int bytesPerLine = avProps.vidProps.nWidth * 4;
  int width = avProps.vidProps.nWidth;
  int height = abs(avProps.vidProps.nHeight);

  // copy, do not use MFormats memory
  QImage image(width, height, QImage::Format_ARGB32);
  memcpy(image.bits(), data, bytesPerLine * height);
  return image;
}

void RendererManager::onTimeout()
{
  CComPtr<IMFFrame> previewFrame;
  previewRenderer_.getFrame(previewFrame);
  QImage previewImage = convertFrame(previewFrame);
  emit onNewPreviewImage(previewImage);

  CComPtr<IMFFrame> programFrame;
  programRenderer_.getFrame(programFrame);
  QImage programImage = convertFrame(programFrame);
  emit onNewProgramImage(programImage);
}

bool RendererManager::reloadProjec()
{
  ProjectManager& pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(!project) return false;

  if(currentProjectUid_ != project->id)
  {
    unload();

    // create source renderers
    for(int i = 0; i < project->scenes.size(); i++)
    {
      for(int j = 0; j < project->scenes[i].sources.size(); j++)
      {
        Source source = project->scenes[i].sources[j];
        if(source.originalId.isNull())
        {
          BaseRenderer *br = BaseRenderer::build(source.type);
          if(br)
          {
            br->setSource(source);
            br->start();
            renderers_.push_back(br);
          }
        }
      }
    }

    // Start scene renderers
    previewRenderer_.start();
    programRenderer_.start();
  }
  else
  {
    QMap<QUuid, Source> sources;
    for(int i = 0; i < project->scenes.size(); i++)
    {
      for(int j = 0; j < project->scenes[i].sources.size(); j++)
      {
        sources.insert(project->scenes[i].sources[j].id, project->scenes[i].sources[j]);
      }
    }

    // unload | update renderes
    for(auto it = renderers_.begin(); it != renderers_.end(); )
    {
      BaseRenderer* renderer = *it;
      if(renderer)
      {
        bool detroyRenderer = !sources.contains(renderer->id());

        if(!detroyRenderer)
        {
          Source source = sources[renderer->id()];
          detroyRenderer = source.dirty;
          if(!detroyRenderer)
          {
            // remove from sources
            sources.remove(renderer->id());

            // next one
            it++;
          }
        }

        // destroy
        if(detroyRenderer)
        {
          // stop and delete
          renderer->stop();
          delete renderer;

          // next one
          it = renderers_.erase(it);
        }
      }
    }

    // create source renderers
    QList<Source> s = sources.values();
    for(int i = 0; i < s.size(); i++)
    {
      Source source = s[i];
      if(source.originalId.isNull())
      {
        BaseRenderer* br = BaseRenderer::build(source.type);
        if(br)
        {
          br->setSource(source);
          br->start();
          renderers_.push_back(br);
        }
      }
    }

    // update current project
    previewRenderer_.update();
    programRenderer_.update();
  }

  // save current project
  currentProjectUid_ = project->id;

  return true;
}

bool RendererManager::unload()
{
  // Stop scene renderers
  previewRenderer_.stop();
  programRenderer_.stop();

  // clear source renderers
  for(int i = 0; i < renderers_.size(); i++)
  {
    BaseRenderer* renderer = renderers_[i];
    if(renderer)
    {
      renderer->stop();
      delete renderer;
    }
  }
  renderers_.clear();

  return true;
}

bool RendererManager::getFrame(QUuid sourceId, CComPtr<IMFFrame>& _frame)
{
  // clear source renderers
  for(int i = 0; i < renderers_.size(); i++)
  {
    BaseRenderer* renderer = renderers_[i];
    if(renderer)
    {
      if(renderer->id() == sourceId)
      {
        renderer->getFrame(_frame);
        break;
      }
    }
  }

  return true;
}

bool RendererManager::startStreaming()
{
  if(streaming_ == EStreamingState::SS_STREAMING || streaming_ == EStreamingState::SS_WAITING_START) return true;

  streaming_ = EStreamingState::SS_WAITING_START;
  emit onStreamingStateChange(streaming_);

  programRenderer_.startStreaming();
  
  return true;
}

bool RendererManager::stopStreaming()
{
  if(streaming_ == EStreamingState::SS_NONE || streaming_ == EStreamingState::SS_WAITING_NONE) return true;

  streaming_ = EStreamingState::SS_WAITING_NONE;
  emit onStreamingStateChange(streaming_);

  programRenderer_.stopStreaming();

  return true;
}

void RendererManager::onStartStreaming()
{
  streaming_ = EStreamingState::SS_STREAMING;
  emit onStreamingStateChange(streaming_);
}

void RendererManager::onStopStreaming()
{
  streaming_ = EStreamingState::SS_NONE;
  emit onStreamingStateChange(streaming_);
}

bool RendererManager::startRecording()
{
  if(recording_ == ERecordingState::RS_RECORDING || recording_ == ERecordingState::RS_WAITING_START) return true;

  recording_ = ERecordingState::RS_WAITING_START;
  emit onRecordingStateChange(recording_);

  programRenderer_.startRecording();

  return true;
}

bool RendererManager::stopRecording()
{
  if(recording_ == ERecordingState::RS_NONE || recording_ == ERecordingState::RS_WAITING_NONE) return true;

  recording_ = ERecordingState::RS_WAITING_NONE;
  emit onRecordingStateChange(recording_);

  programRenderer_.stopRecording();

  return true;
}

void RendererManager::onStartRecording()
{
  recording_ = ERecordingState::RS_RECORDING;
  emit onRecordingStateChange(recording_);
}

void RendererManager::onStopRecording()
{
  recording_ = ERecordingState::RS_NONE;
  emit onRecordingStateChange(recording_);
}


