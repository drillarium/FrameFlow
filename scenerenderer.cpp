#include "scenerenderer.h"
#include "project_manager.h"
#include "renderermanager.h"
#include <QElapsedTimer>
#include <chrono>
#include <QDir>

using namespace std::chrono_literals;

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
      // CComBSTR ttype(L"slide(slideStyle='SWAP' bands='100')");
      CComBSTR propList;
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
    rm.getFrame(scene.sources[i].originalId.isNull()? scene.sources[i].id : scene.sources[i].originalId, sourceFrame);
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

  // commandThread
  std::thread commandThread = std::thread([&] { nextCommandThread(); });

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

    // streaming
    if(streamingWriterOpened_)
    {
      if(streamingWriter_)
      {
        std::lock_guard<std::mutex> lock(streamingWriterMutex_);
        
        REFERENCE_TIME maxWait = -1;
        CComBSTR props;
        CComQIPtr<IMFReceiver> receiverPreview(streamingWriter_);
        receiverPreview->ReceiverFramePut(frame, maxWait, props);
      }
    }
    // file
    if(fileWriterOpened_)
    {
      if(fileWriter_)
      {
        std::lock_guard<std::mutex> lock(fileWriterMutex_);

        REFERENCE_TIME maxWait = -1;
        CComBSTR props;
        CComQIPtr<IMFReceiver> receiverPreview(fileWriter_);
        receiverPreview->ReceiverFramePut(frame, maxWait, props);
      }
    }

    // preview
    REFERENCE_TIME maxWait = -1;
    CComBSTR props;
    CComQIPtr<IMFReceiver> receiverPreview(preview);
    receiverPreview->ReceiverFramePut(frame, maxWait, props);
  }

cleanup:
  {
    if(commandThread.joinable())
    {
      commandThread.join();
    }
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

int get_writer_index(IMFWriter* writer, eMFWriterOption option, const char* _name)
{
  if(!writer) return -1;

  int count = 0;
  writer->WriterOptionGetCount(option, &count);
  for(int i = 0; i < count; i++)
  {
    CComBSTR name, longName;
    writer->WriterOptionGetByIndex(option, i, &name, &longName);
    if(longName)
    {
      std::wstring wideLong(longName);
      std::string strLong(wideLong.begin(), wideLong.end());
      if(!strLong.compare(_name)) return i;
    }
    if(name)
    {
      std::wstring wide(name);
      std::string str(wide.begin(), wide.end());
      if(!str.compare(_name)) return i;
    }
  }

  return -1;
}

bool has_codec(const char* codec)
{
  CComPtr<IMFWriter> writer;
  HRESULT hr = writer.CoCreateInstance(__uuidof(MFWriter));
  if(FAILED(hr))
    return false;

  int index = get_writer_index(writer, eMFWO_Format, "mpegts");
  if(index < 0)
    return false;

  CComPtr<IMFProps> props;
  hr = writer->WriterOptionSetByIndex(eMFWO_Format, index, &props);
  if(FAILED(hr))
    return false;

  int codecIndex = get_writer_index(writer, eMFWO_VideoCodec, codec);
  return (codecIndex >= 0);
}

#define H264_NVIDIA_CODED "n264"
#define H264_QS_SW_CODEC "q264sw"
#define H264_QS_HW_CODEC "q264hw"
#define H264_CISCO_CODEC "libopenh264"

QString get_h264_codec()
{
  if(has_codec(H264_NVIDIA_CODED))     return H264_NVIDIA_CODED;
  else if(has_codec(H264_QS_HW_CODEC)) return H264_QS_HW_CODEC;
  else if(has_codec(H264_QS_SW_CODEC)) return H264_QS_SW_CODEC;
  return H264_CISCO_CODEC;
}

void SceneRenderer::nextCommandThread()
{
  while(running_)
  {
    // next command
    ECommand nextCommand = nextCommand_;
    nextCommand_ = ECommand::CMD_NONE;
  
    if(nextCommand == ECommand::CMD_START_STREAMING)
    {
      if(!streamingWriter_)
      {
        // open
        HRESULT hr = streamingWriter_.CoCreateInstance(__uuidof(MFWriter));
        if(SUCCEEDED(hr))
        {
          ProjectManager &pm = ProjectManager::instance();
          auto ssl = pm.listStreamingServers();
          for(StreamingServer ss : ssl)
          {
            if(ss.enabled)
            {
              // youyube
              QString qurl = QString("%1/%2").arg(ss.url).arg(ss.key);
              CComBSTR url = qurl.toStdWString().c_str();
              QString h264Codec = get_h264_codec();
              QString qcondif = QString(" format='flv' protocol='rtmp://' video::codec='%1' audio::codec='libmp3lame' audio::ar='44100'").arg(h264Codec);
              CComBSTR config = qcondif.toStdWString().c_str();
              int reset = 1;
              hr = streamingWriter_->WriterSet(url, reset, config);
            }
            break;
          }
        }

        streamingWriterOpened_ = true;
        emit onStartStreaming();
      }
    }
    else if(nextCommand == ECommand::CMD_STOP_STREAMING)
    {
      streamingWriterOpened_ = false;

      {
        std::lock_guard<std::mutex> lock(streamingWriterMutex_);
        if(streamingWriter_)
        {
          BOOL async = FALSE;
          streamingWriter_->WriterClose(async);
          streamingWriter_ = NULL;
        }
      }

      emit onStopStreaming();
    }
    else if(nextCommand == ECommand::CMD_START_RECORDING)
    {
      if(!fileWriter_)
      {
        // open
        HRESULT hr = fileWriter_.CoCreateInstance(__uuidof(MFWriter));
        if(SUCCEEDED(hr))
        {
          ProjectManager &pm = ProjectManager::instance();
          EncoderSettings encoderSettings = pm.encoderSettings();

          QDir dir(encoderSettings.outputFolder);
          if(!dir.exists()) dir.mkpath(".");
          QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
          QString fileName = QString("FrameFlow_%1.mp4").arg(timestamp);
          QString fullPath = dir.filePath(fileName);

          CComBSTR url = fullPath.toStdWString().c_str();
          QString h264Codec = get_h264_codec();
          QString qcondif = QString(" format='mp4' video::codec='%1' audio::codec='aac' audio::ar='44100' audio::b='128K'").arg(h264Codec);
          CComBSTR config = qcondif.toStdWString().c_str();
          int reset = 1;
          hr = fileWriter_->WriterSet(url, reset, config);
        }

        fileWriterOpened_ = true;
        emit onStartRecording();
      }
    }
    else if(nextCommand == ECommand::CMD_STOP_RECORDING)
    {
      fileWriterOpened_ = false;

      {
        std::lock_guard<std::mutex> lock(fileWriterMutex_);
        if(fileWriter_)
        {
          BOOL async = FALSE;
          fileWriter_->WriterClose(async);
          fileWriter_ = NULL;
        }
      }

      emit onStopRecording();
    }
    
    std::this_thread::sleep_for(1ms);
  }

  if(streamingWriter_)
  {
    BOOL async = FALSE;
    streamingWriter_->WriterClose(async);
    streamingWriter_ = NULL;
  }

  if(fileWriter_)
  {
    BOOL async = FALSE;
    fileWriter_->WriterClose(async);
    fileWriter_ = NULL;
  }
}
