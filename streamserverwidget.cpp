#include "streamserverwidget.h"

StreamServerWidget::StreamServerWidget(const StreamingServer& _ss, QWidget *parent)
:QWidget(parent)
,streamingServer_(_ss)
{
  ui.setupUi(this);

  ui.comboBox->setCurrentText(streamingServer_.platform);
  ui.nameLineEdit->setText(streamingServer_.name);
  ui.serverLineEdit->setText(streamingServer_.url);
  ui.streamKeyLineEdit->setText(streamingServer_.key);
  ui.onOffSwitch->setChecked(streamingServer_.enabled);

  connect(ui.removeButton, &QPushButton::clicked, this, &StreamServerWidget::onRemoveStreamServer);

  connect(ui.eyeButton, &QPushButton::toggled, this,
    [this](bool checked) {
      ui.streamKeyLineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
      ui.eyeButton->setIcon(QIcon(checked ? ":/FrameFlow/eye-slash.svg" : ":/FrameFlow/eye.svg"));
  });
}

StreamServerWidget::~StreamServerWidget()
{
}
