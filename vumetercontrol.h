#pragma once

#include <QWidget>
#include "ui_vumetercontrol.h"
#include "MFormats.h"

// VuMeterWidget
class VumeterControl : public QWidget
{
Q_OBJECT

public:
  VumeterControl(QWidget *_parent = nullptr);
  ~VumeterControl();

  void setDevice(const QString &_name);
  void setAudioLoudness(const M_AUDIO_LOUDNESS &_al);

protected slots:
  void onVolumeChange(int value);

signals:
  void onVumeterValueChanged(double);

private:
  Ui::VumeterControlClass ui;
};
