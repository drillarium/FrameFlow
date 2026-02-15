#include "sourcereaderbaserenderer.h"
#include "project_manager.h"
#include <chrono>
#include <QImage>
#include <QFileInfo>
#include <QMovie>

using namespace std::chrono_literals;

QStringList SourceReaderBaseRenderer::listOfLiveSources()
{
  QStringList ret;

  CComPtr<IMFReader> reader;
  reader.CoCreateInstance(__uuidof(MFReader));

  CComQIPtr<IMSenders> senders(reader);
  int numSenders = 0;
  senders->SendersGetCount(&numSenders);
  for(int i = 0; i < numSenders; ++i)
  {
    CComBSTR name;
    senders->SendersGetByIndex(i, &name, NULL, NULL);
    QString str = QString::fromWCharArray(name);
    ret.push_back(str);
  }
  reader->ReaderClose();

  return ret;
}

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

bool SourceReaderBaseRenderer::getFrame(CComPtr<IMFFrame>& _frame)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) return false;

  lastFrame_->MFClone(&_frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);

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

  if(_source.config.contains("line") && _source.config.value("line").isString())
  {
    url_ = QString("mp://%1").arg(_source.config.value("line").toString()).toStdString();
  }

  if(_source.config.contains("loop") && _source.config.value("loop").isBool())
  {
    loop_ = _source.config.value("loop").toBool();
  }

  return true;
}

bool isImageLocalUrl(const QString& url)
{
  static const QStringList validExtensions = { "png", "jpg", "jpeg", "bmp", "gif", "webp", "svg" };

  QString lower = url.toLower();

  for(const QString& ext : validExtensions)
  {
    if(lower.endsWith("." + ext))
      return true;
  }

  return false;
}

bool isImageRemoteUrl(const QUrl& url)
{
  if(!url.isValid()) return false;

  static const QStringList validExtensions = { "png", "jpg", "jpeg", "bmp", "gif", "webp", "svg" };

  QString path = url.path();                 // removes query params
  QString suffix = QFileInfo(path).suffix().toLower();

  return validExtensions.contains(suffix);
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

  QImage imageARGB;
  bool useReader = true;
  if(type_ == SourceType::EST_FILE || type_ == SourceType::EST_URL) useReader = !( isImageLocalUrl(QString::fromStdString(url_)) || isImageRemoteUrl(QString::fromStdString(url_)) );
  QMovie* movie = nullptr;

  CComPtr<IMFFactory> factory;
  CComBSTR colorParameters = L"solid_color='Black(0)'";
  uint32_t samples = (aProps.nSamplesPerSec * 1) / 25;
  CComPtr<IMFFrame> blackFrame;
  M_AV_PROPS avPropsBlack = avProps;

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
  if(!useReader)
  {
    avPropsBlack.vidProps.fccType = eMFCC::eMFCC_ARGB32;
    hr = factory->MFFrameCreateFromMem(&avPropsBlack, 0, (long) samples, (LONGLONG) 0, &blackFrame, colorParameters);
    if(FAILED(hr) && !blackFrame) goto cleanup;
    else if(project)
    {
      blackFrame->MFAVPropsGet(&avProps, NULL);
      M_TIME mTime = {};
      blackFrame->MFTimeGet(&mTime);
      int fpsNum = 0, fpsDen = 0;
      getFactors(project->framerate, fpsNum, fpsDen);
      REFERENCE_TIME timePerFrame = REFERENCE_TIME(((fpsDen * 10000000.f) / fpsNum) + 0.5);
      mTime.rtEndTime = mTime.rtStartTime + timePerFrame;
      blackFrame->MFTimeSet(&mTime);
    }
  }

  // reader
  if(useReader)
  {
    hr = reader.CoCreateInstance(__uuidof(MFReader));
    if(FAILED(hr)) goto cleanup;
  }

  while(running_)
  {
    if(openReader)
    {
      if(useReader)
      {
        reader->ReaderClose();
        std::wstring path(url_.begin(), url_.end());
        CComBSTR pathw(path.c_str());
        CComBSTR propList;
        openReader = FAILED(reader->ReaderOpen(pathw, propList));
      }
      else
      {
        if(movie) delete movie;
        movie = new QMovie(QString::fromStdString(url_));      
        if(movie->isValid() && movie->frameCount() > 1)
        {
          movie->start();
          movie->setPaused(true);
          openReader = false;
        }
        else
        {
          delete movie, movie = nullptr;
          QImage image(QString::fromStdString(url_));
          if(!image.isNull())
          {
            imageARGB = image.convertToFormat(QImage::Format_ARGB32).scaled(avProps.vidProps.nWidth, abs(avProps.vidProps.nHeight));
            openReader = imageARGB.isNull();
          }
        }
      }
    }

    CComPtr<IMFFrame> frame;
    if(!openReader)
    {
      // read
      if(useReader)
      {
        CComQIPtr<IMFSource> source(reader);
        REFERENCE_TIME maxWait = -1;
        CComBSTR hints;
        hr = source->SourceFrameConvertedGetByNumber(&avProps, frameNumber, maxWait, &frame, hints);     
        openReader = !frame;
      }
      else
      {
        blackFrame->MFClone(&frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);

        if(movie)
        {
          imageARGB = movie->currentImage().convertToFormat(QImage::Format_ARGB32).scaled(avProps.vidProps.nWidth, abs(avProps.vidProps.nHeight));
          if(!movie->jumpToNextFrame())    
          {
            if(loop_) movie->jumpToFrame(0);
          }
        }

        // copy to IMFrame
        if(!imageARGB.isNull())
        {
          MF_VID_PTR vPtr;
          frame->MFVideoGetBytesEx(&vPtr);

          BYTE* pData = (BYTE *) (vPtr.lpVideoPlanes[0]);
          const uchar* src = imageARGB.constBits();
          memcpy(pData, src, vPtr.cbVideoRowBytes[0] * imageARGB.height());
        }
      }

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

    if(movie)
    {
      delete movie;
    }
  }
}