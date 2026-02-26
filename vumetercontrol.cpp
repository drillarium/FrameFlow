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
    else ui.vumeterWidget->setLevelDb(-100);
  }
  else
  {
    ui.vumeterWidget->setLevelDb(-100);
  }
}

void VumeterControl::onMute()
{
  bool mute = ui.muteButton->isChecked();
  double value = mute? 0 : ui.horizontalSlider->value() / 100.;
  emit onVumeterValueChanged(value);
}

void VumeterControl::onVolumeChange(int value)
{
  ui.muteButton->setChecked(value == 0);
  onMute();
}

