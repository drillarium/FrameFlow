#include "scenerenderer.h"

SceneRenderer::SceneRenderer(QObject* parent)
:QObject(parent)
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
  CComBSTR channel;
  BOOL enableVideo = FALSE;
  BOOL enableAudio = FALSE;

  CComPtr<IMFFactory> factory;
  CComBSTR colorParameters = L"solid_color='Red(255)'";
  M_VID_PROPS vProps = { eMVF_HD1080_25p };
  M_AUD_PROPS aProps = { 2, 48000, 16, 0 };
  M_AV_PROPS avProps = { vProps, aProps };
  uint32_t samples = (aProps.nSamplesPerSec * 1) / 25;
  CComPtr<IMFFrame> blackFrame;

  // coinit
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if(FAILED(hr)) goto cleanup;

  // preview
  hr = preview_.CoCreateInstance(__uuidof(MFPreview));
  if(FAILED(hr)) goto cleanup;
  preview_->PreviewEnable(channel, enableAudio, enableVideo);

  // factory
  hr = factory.CoCreateInstance(__uuidof(MFFactory));
  if(FAILED(hr)) goto cleanup;

  // blackframe
  hr = factory->MFFrameCreateFromMem(&avProps, 0, (long) samples, (LONGLONG) 0, &blackFrame, colorParameters);
  if(FAILED(hr)) goto cleanup;

  while(running_)
  {
    // convert to RGB
    CComPtr<IMFFrame> ARGBFrame;
    M_AV_PROPS ARGBProps = avProps;
    ARGBProps.vidProps.fccType = eMFCC::eMFCC_ARGB32;
    int frameRest = 0;
    CComBSTR converterProps;
    CComBSTR converter;
    blackFrame->MFConvert(&ARGBProps, &ARGBFrame, &frameRest, converterProps, converter);
  
    if(ARGBFrame)
    {
      M_AV_PROPS avProps = {};
      ARGBFrame->MFAVPropsGet(&avProps, NULL);
      MF_VID_PTR vPtr;
      ARGBFrame->MFVideoGetBytesEx(&vPtr);

      QImage img;
      int bytesPerLine = avProps.vidProps.nWidth * 4;
      int height = abs(avProps.vidProps.nHeight);
      if(avProps.vidProps.nHeight > 0)
      {
        img = QImage((uint8_t *) vPtr.lpVideoPlanes[0], avProps.vidProps.nWidth, height, bytesPerLine, QImage::Format_ARGB32);
      }
      else
      {
        img = QImage((uint8_t*) vPtr.lpVideoPlanes[0], avProps.vidProps.nWidth, height, bytesPerLine, QImage::Format_ARGB32);
        img = img.mirrored(false, true);
      }

      QMetaObject::invokeMethod(this, [this, img]() { onNewImage(img); });
    }

    // preview
    REFERENCE_TIME maxWait = -1;
    CComBSTR props;
    CComQIPtr<IMFReceiver> receiverPreview(preview_);
    receiverPreview->ReceiverFramePut(blackFrame, maxWait, props);
  }

cleanup:
  {

  }
}
