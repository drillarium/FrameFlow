#include "vumetercontrol.h"

// VumeterControl
VumeterControl::VumeterControl(QWidget *_parent)
:QWidget(_parent)
{
  ui.setupUi(this);
}

VumeterControl::~VumeterControl()
{

}

void VumeterControl::setDevice(const QString& _name)
{
  ui.titleLabel->setText(_name);
  ui.titleLabel->setToolTip(_name);
}

bool is_infinite(float value)
{
  float max_value = (std::numeric_limits<float>::max)();
  float min_value = -max_value;
  return !(min_value <= value) && (value <= max_value);
}

void VumeterControl::setAudioLoudness(const M_AUDIO_LOUDNESS& _al)
{
  if(_al.nValidTracks >= 1)
  {
    bool valid = !is_infinite(_al.arrVUMeter[0]);
    if(valid) ui.vumeterWidget->setLevelDb(_al.arrVUMeter[0]);
    else ui.vumeterWidget->setLevelDb(-60);
  }
  else
  {
    ui.vumeterWidget->setLevelDb(-60);
  }
}

void VumeterControl::onVolumeChange(int)
{
  int v = ui.horizontalSlider->value();
  ui.volumeLabel->setText(QString::number(v));
  double value = ui.horizontalSlider->value() / 100.;
  emit onVumeterValueChanged(value);
}

