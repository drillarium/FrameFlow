#include "sourcereaderbaserenderer.h"
#include "project_manager.h"
#include <chrono>

using namespace std::chrono_literals;

SourceReaderBaseRenderer::SourceReaderBaseRenderer(SourceType _type)
:BaseRenderer()
,type_(_type)
{

}

SourceReaderBaseRenderer::~SourceReaderBaseRenderer()
{
  stop();
}

bool SourceReaderBaseRenderer::start()
{
  stop();
  workerThread_ = std::thread([&] { running_ = true; workerThread(); });
  return true;
}

bool SourceReaderBaseRenderer::stop()
{
  if(workerThread_.joinable())
  {
    running_ = false;
    workerThread_.join();
  }
  return true;
}

bool SourceReaderBaseRenderer::getFrame(CComPtr<IMFFrame>& _frame, QRect& _rect)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) return false;

  lastFrame_->MFClone(&_frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);
  _rect = rect_;

  return true;
}

bool SourceReaderBaseRenderer::setSource(Source _source)
{
  BaseRenderer::setSource(_source);

  if(_source.config.contains("url") && _source.config.value("url").isString())
  {
    url_ = _source.config.value("url").toString().toStdString();
  }

  if(_source.config.contains("path") && _source.config.value("path").isString())
  {
    url_ = _source.config.value("path").toString().toStdString();
  }

  if(_source.config.contains("loop") && _source.config.value("loop").isBool())
  {
    loop_ = _source.config.value("loop").toBool();
  }

  rect_ = getSourceRect(_source);

  return true;
}

void SourceReaderBaseRenderer::workerThread()
{
  CComPtr<IMPreview> preview;
  CComBSTR channel;
  BOOL enableVideo = FALSE;
  BOOL enableAudio = FALSE;

  M_VID_PROPS vProps = { eMVF_HD1080_25p };
  M_AUD_PROPS aProps = { 2, 48000, 16, 0 };
  M_AV_PROPS avProps = { vProps, aProps };

  CComPtr<IMFReader> reader;
  bool openReader = true;
  int frameNumber = -1;

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

  // reader
  hr = reader.CoCreateInstance(__uuidof(MFReader));
  if(FAILED(hr)) goto cleanup;

  while(running_)
  {
    CComPtr<IMFFrame> frame;

    if(openReader)
    {
      reader->ReaderClose();
      std::wstring path(url_.begin(), url_.end());
      CComBSTR pathw(path.c_str());
      CComBSTR propList;
      openReader = FAILED(reader->ReaderOpen(pathw, propList));
    }

    if(!openReader)
    {
      // read
      CComQIPtr<IMFSource> source(reader);
      REFERENCE_TIME maxWait = -1;
      CComBSTR hints;
      hr = source->SourceFrameConvertedGetByNumber(&avProps, frameNumber, maxWait, &frame, hints);     
      openReader = !frame;

      if(frame)
      {
        // loop
        M_TIME timecode;
        frame->MFTimeGet(&timecode);

        // check last
        if(type_ == SourceType::EST_FILE)
        {
          frameNumber = -1;
          bool last = !!(timecode.eFFlags & eMFF_Last);
          if(last && loop_) frameNumber = 0;
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
    }
    if(!frame)
    {
      std::this_thread::sleep_for(1s);
    }
  }

cleanup:
  {
    preview = NULL;
    if(reader)
    {
      reader->ReaderClose();
      reader = NULL;
    }
  }
}