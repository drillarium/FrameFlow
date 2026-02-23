#include "settingsdialog.h"
#include <QKeyEvent>
#include "streamserverwidget.h"
#include "project_manager.h"
#include "confirmationdialog.h"
#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *_parent)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  ui.streamServersListWidgets->setFixedHeight(0);

  ProjectManager &pm = ProjectManager::instance();
  auto ss = pm.listStreamingServers();

  for(StreamingServer s : ss)
  {
    addStreamingServer(s);
  }

  // load
  auto settings = pm.encoderSettings();
  loadEncoderSettings(settings);
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Escape)
  {
    event->ignore();
    return;
  }
  QDialog::keyPressEvent(event);
}

void SettingsDialog::onAddStreamServer()
{
  StreamingServer ss;
  ss.name = "New Streaming Server";
  ss.platform = "Custom RTMP";
  ss.enabled = true;

  ProjectManager& pm = ProjectManager::instance();
  if(pm.addStreamingServer(ss))
  {
    addStreamingServer(ss);
  }
}

void SettingsDialog::addStreamingServer(const StreamingServer &_ss)
{
  QListWidgetItem* lwi = new QListWidgetItem(ui.streamServersListWidgets);
  lwi->setSizeHint(QSize(0, 205));
  StreamServerWidget* ssw = new StreamServerWidget(_ss);
  auto lw = ui.streamServersListWidgets;
  connect(ssw, &StreamServerWidget::onRemoveStreamServer, this, [&, lw, lwi]() {
    // confirmation
    QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Remove Stream Server", "Are you sure you want to remove this server?", QMessageBox::Yes, QMessageBox::No, QMessageBox::No);
    if(reply != QMessageBox::Yes) return;

    StreamServerWidget *widget = static_cast<StreamServerWidget*>(lw->itemWidget(lwi));
    if(!widget) return;

    QUuid id = widget->id();

    // save db
    ProjectManager &pm = ProjectManager::instance();
    if(!pm.removeStreamingServer(id)) return;

    // remove from list
    int row = lw->row(lwi);
    QListWidgetItem* it = lw->takeItem(row);
    delete it;
    lw->setFixedHeight((205 * lw->count()) + (6 * lw->count()));
  });
  connect(ssw, &StreamServerWidget::onSaveStreamServer, this, [&, lw, lwi]() {
    StreamServerWidget *widget = static_cast<StreamServerWidget*>(lw->itemWidget(lwi));
    if(!widget) return;

    StreamingServer server = widget->server();

    // save db
    ProjectManager &pm = ProjectManager::instance();
    if(!pm.updateStreamingServer(server)) return;
  });
  ui.streamServersListWidgets->addItem(lwi);
  ui.streamServersListWidgets->setItemWidget(lwi, ssw);

  ui.streamServersListWidgets->setFixedHeight((205 * ui.streamServersListWidgets->count()) + (6 * ui.streamServersListWidgets->count()));
}

QString videoEncoderText(const QString &_videoEncoder)
{
  if(_videoEncoder == "NVIDIA NVENC H.264") return "nvenc";
  if(_videoEncoder == "NVIDIA NVENC HEVC") return "nvenc";
  if(_videoEncoder == "x264 (CPU)") return "nvenc";
  if(_videoEncoder == "AMD AMF H.264") return "nvenc";
  return "";
}

QString rateControlText(const QString& _rateControl)
{
  if(_rateControl == "CBR (Constant Bitrate)") return "cbr";
  if(_rateControl == "VBR (Variable Bitrate)") return "vbr";
  if(_rateControl == "CQP (Constant Quality)") return "cqp";
  return "";
}

QString audioEncoderText(const QString& _audioEncoder)
{
  if(_audioEncoder == "AAC") return "aac";
  if(_audioEncoder == "Opus") return "opus";
  if(_audioEncoder == "MP3") return "mp3";
  return "";
}

int keyFrameIntervalInt(const QString& _keyFrame)
{
  if(_keyFrame == "1 second") return 1;
  if(_keyFrame == "2 seconds") return 2;
  if(_keyFrame == "3 seconds") return 3;
  if(_keyFrame == "4 seconds") return 4;
  return 0;
}

int audioBitrateInt(const QString& _bitrate)
{
  if(_bitrate == "96 Kbps") return 96;
  if(_bitrate == "128 Kbps") return 128;
  if(_bitrate == "160 Kbps") return 160;
  if(_bitrate == "192 Kbps") return 192;
  if(_bitrate == "256 Kbps") return 256;
  if(_bitrate == "320 Kbps") return 320;
  return 0;
}

int audioSampleRateInt(const QString& _rate)
{
  if(_rate == "48 KHz") return 48000;
  if(_rate == "44.1 KHz") return 44100;
  return 0;
}

int videoCodecIndex(const QString& _codec)
{
  if(_codec == "nvenc") return 0;
  if(_codec == "nvenc") return 1;
  if(_codec == "nvenc") return 2;
  if(_codec == "nvenc") return 3;
  return 0;
}

int rateControlIndex(const QString& _rate)
{
  if(_rate == "cbr") return 0;
  if(_rate == "vbr") return 1;
  if(_rate == "cqp") return 2;
  return 0;
}

int keyFrameIndex(int _key)
{
  if(_key == 1) return 0;
  if(_key == 2) return 1;
  if(_key == 3) return 2;
  if(_key == 4) return 3;
  return 0;
}

int audioCodecIndex(const QString& _codec)
{
  if(_codec == "aac") return 0;
  if(_codec == "opus") return 1;
  if(_codec == "mp3") return 2;
  return 0;
}

int audioBitrateIndex(int _bitrate)
{
  if(_bitrate == 96) return 0;
  if(_bitrate == 128) return 128;
  if(_bitrate == 160) return 160;
  if(_bitrate == 192) return 192;
  if(_bitrate == 256) return 256;
  if(_bitrate == 320) return 320;
  return 0;
}

int sampleRateIndex(int _rate)
{
  if(_rate == 48000) return 0;
  if(_rate == 44100) return 1;
  return 0;
}

void SettingsDialog::loadEncoderSettings(const EncoderSettings& _settings)
{
  ui.videoEncoderComboBox->setCurrentIndex(videoCodecIndex(_settings.videoEncoder));
  ui.videoBitrateSlider->setValue(_settings.videoBitrateKbps);
  ui.rateControlComboBox->setCurrentIndex(rateControlIndex(_settings.rateControl));
  ui.keyFrameIntervalComboBox->setCurrentIndex(keyFrameIndex(_settings.keyFrameIntervalInSeconds));
  ui.audioEncoderComboBox->setCurrentIndex(audioCodecIndex(_settings.audioEncoder));
  ui.audioBitrateComboBox->setCurrentIndex(audioBitrateIndex(_settings.audioBitrateKbps));
  ui.sampleRateComboBox->setCurrentIndex(sampleRateIndex(_settings.sampleRate));
  ui.outputFolderLineEdit->setText(_settings.outputFolder);
  onVideoBitrateChange();
}

void SettingsDialog::onSaveEncoding()
{
  EncoderSettings settings;

  settings.videoEncoder = videoEncoderText(ui.videoEncoderComboBox->currentText());
  settings.videoBitrateKbps = ui.videoBitrateSlider->value();
  settings.rateControl = rateControlText(ui.rateControlComboBox->currentText());
  settings.keyFrameIntervalInSeconds = keyFrameIntervalInt(ui.keyFrameIntervalComboBox->currentText());
  settings.audioEncoder = audioEncoderText(ui.audioEncoderComboBox->currentText());
  settings.audioBitrateKbps = audioBitrateInt(ui.audioBitrateComboBox->currentText());
  settings.sampleRate = audioSampleRateInt(ui.sampleRateComboBox->currentText());
  settings.outputFolder = ui.outputFolderLineEdit->text();

  emit saveEncoding(settings);
}

void SettingsDialog::onSelectOutputFolder()
{
  QString initialFolder = ui.outputFolderLineEdit->text();
  if(initialFolder.isEmpty()) initialFolder = QDir::homePath();
  QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", initialFolder, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if(!folderPath.isEmpty())
  {
    ui.outputFolderLineEdit->setText(folderPath);
  }
}

void SettingsDialog::onVideoBitrateChange()
{
  int kbps = ui.videoBitrateSlider->value();
  ui.bitrateLabel->setText(QString("%1 Kbps").arg(kbps));
}
