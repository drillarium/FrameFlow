#pragma once

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

struct EncoderSettings
{
  QString videoEncoder = "nvenc";
  int videoBitrateKbps = 3000;
  QString rateControl = "CBR";
  int keyFrameIntervalInSeconds = 1;
  QString audioEncoder = "aac";
  int audioBitrateKbps = 96;
  int sampleRate = 48000;
  QString outputFolder;
};

static EncoderSettings modelFromJSON(const QJsonDocument &_doc)
{
  EncoderSettings settings;

  QJsonObject obj = _doc.object();
  settings.videoEncoder = obj["video_codec"].toString();
  settings.videoBitrateKbps = obj["video_bitrate"].toInt();
  settings.rateControl = obj["video_rate_control"].toString();
  settings.keyFrameIntervalInSeconds = obj["video_key_frame_interval"].toInt();
  settings.audioEncoder = obj["audio_codec"].toString();
  settings.audioBitrateKbps = obj["audio_bitrate"].toInt();
  settings.sampleRate = obj["audio_sample_rate"].toInt();
  settings.outputFolder = obj["output_folder"].toString();

  return settings;
}

static QJsonDocument JSONfromModel(const EncoderSettings &_settings)
{
  QJsonObject obj;
  obj["video_codec"] = _settings.videoEncoder;
  obj["video_bitrate"] = _settings.videoBitrateKbps;
  obj["video_rate_control"] = _settings.rateControl;
  obj["video_key_frame_interval"] = _settings.keyFrameIntervalInSeconds;
  obj["audio_codec"] = _settings.audioEncoder;
  obj["audio_bitrate"] = _settings.audioBitrateKbps;
  obj["audio_sample_rate"] = _settings.sampleRate;
  obj["output_folder"] = _settings.outputFolder;

  return QJsonDocument(obj);
}