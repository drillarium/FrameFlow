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

  QImage img;
  int bytesPerLine = avProps.vidProps.nWidth * 4;
  int height = abs(avProps.vidProps.nHeight);
  if(avProps.vidProps.nHeight > 0)
  {
    img = QImage((uint8_t*)vPtr.lpVideoPlanes[0], avProps.vidProps.nWidth, height, bytesPerLine, QImage::Format_ARGB32);
  }
  else
  {
    img = QImage((uint8_t*)vPtr.lpVideoPlanes[0], avProps.vidProps.nWidth, height, bytesPerLine, QImage::Format_ARGB32);
    // img = img.mirrored(false, true);
  }

  return img;
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
  unload();

  ProjectManager &pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(!project) return false;

  // create source renderers
  for(int i = 0; i < project->scenes.size(); i++)
  {
    for(int j = 0; j < project->scenes[i].sources.size(); j++)
    {
      Source source = project->scenes[i].sources[j];
      BaseRenderer *br = BaseRenderer::build(source.type);
      if(br)
      {
        br->setSource(source);
        br->start();
        renderers_.push_back(br);
      }
    }
  }

  // Start scene renderers
  previewRenderer_.start();
  programRenderer_.start();

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

bool RendererManager::getFrame(QUuid sourceId, CComPtr<IMFFrame>& _frame, QRect &_rect)
{
  // clear source renderers
  for(int i = 0; i < renderers_.size(); i++)
  {
    BaseRenderer* renderer = renderers_[i];
    if(renderer)
    {
      if(renderer->id() == sourceId)
      {
        renderer->getFrame(_frame, _rect);
        break;
      }
    }
  }

  return true;
}

