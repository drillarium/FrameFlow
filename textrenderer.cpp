#include "textrenderer.h"
#include "project_manager.h"

TextRenderer::TextRenderer()
:BaseRenderer()
{

}

TextRenderer::~TextRenderer()
{
  stop();
}

bool TextRenderer::start()
{
  stop();
  workerThread_ = std::thread([&] { running_ = true; workerThread(); });
  return true;
}

bool TextRenderer::stop()
{
  if(workerThread_.joinable())
  {
    running_ = false;
    workerThread_.join();
  }
  return true;
}

bool TextRenderer::setSource(Source _source)
{
  BaseRenderer::setSource(_source);

  if(_source.config.contains("font") && _source.config.value("font").isString())
  {
    font_ = _source.config.value("font").toString();
  }
  if(_source.config.contains("text") && _source.config.value("text").isString())
  {
    text_ = _source.config.value("text").toString();
  }
  if(_source.config.contains("color") && _source.config.value("color").isString())
  {
    color_ = QColor(_source.config.value("color").toString());
  }

  return true;
}

#include <QImage>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QColor>

QImage createTextImage(
  const QSize& size,
  const QString& text,
  const QString& fontFamily,
  const QColor& color)
{
  // Create transparent image
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);

  // Start with a large font size (we'll shrink if needed)
  int fontSize = size.height();
  QFont font(fontFamily);
  font.setPixelSize(fontSize);

  QRect targetRect = image.rect();

  // Shrink font until text fits inside image
  while(fontSize > 1) {
    font.setPixelSize(fontSize);
    QFontMetrics fm(font);

    QRect textRect = fm.boundingRect(targetRect,
      Qt::AlignCenter,
      text);

    if(textRect.width() <= size.width() &&
      textRect.height() <= size.height())
    {
      break;
    }

    fontSize--;
  }

  painter.setFont(font);
  painter.setPen(color);

  painter.drawText(targetRect,
    Qt::AlignCenter | Qt::TextWordWrap,
    text);

  painter.end();

  return image;
}

void TextRenderer::workerThread()
{
  CComPtr<IMPreview> preview;
  CComBSTR channel;
  BOOL enableVideo = FALSE;
  BOOL enableAudio = FALSE;

  CComPtr<IMFFactory> factory;
  CComBSTR colorParameters= L"solid_color='Black(0)'";
  M_VID_PROPS vProps = { eMVF_HD1080_25p };
  M_AUD_PROPS aProps = { 2, 48000, 16, 0 };
  M_AV_PROPS avProps = { vProps, aProps };
  uint32_t samples = (aProps.nSamplesPerSec * 1) / 25;
  CComPtr<IMFFrame> blackFrame;

  QImage image;
  int width = 0, height = 0;

  // from current project
  ProjectManager& pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(project)
  {
    QRect r = getSourceRect(source_);
    width = project->width; // r.width();
    height = project->height; // r.height();

    avProps.vidProps = getMVideoFormat(width, height, project->framerate);
    avProps.vidProps.fccType = eMFCC::eMFCC_ARGB32;
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

  // create image and copy to frame
  image = createTextImage(QSize(width, height), text_, font_, color_);
  if(!image.isNull())
  {
    MF_VID_PTR vPtr;
    blackFrame->MFVideoGetBytesEx(&vPtr);

    BYTE* pData = (BYTE*)(vPtr.lpVideoPlanes[0]);
    const uchar* src = image.constBits();
    memcpy(pData, src, vPtr.cbVideoRowBytes[0] * image.height());
  }

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

bool TextRenderer::getFrame(CComPtr<IMFFrame>& _frame)
{
  std::lock_guard<std::mutex> lock(lastFrameMutex_);
  if(!lastFrame_) return false;

  lastFrame_->MFClone(&_frame, eMFrameClone::eMFC_Full, eMFCC::eMFCC_Default);

  return true;
}
