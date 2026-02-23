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
}

StreamServerWidget::~StreamServerWidget()
{
}

void StreamServerWidget::onSave()
{
  streamingServer_.platform = ui.comboBox->currentText();
  streamingServer_.name = ui.nameLineEdit->text();
  streamingServer_.url = ui.serverLineEdit->text();
  streamingServer_.key = ui.streamKeyLineEdit->text();
  streamingServer_.enabled = ui.onOffSwitch->isChecked();

  emit onSaveStreamServer();
}

void StreamServerWidget::onRemove()
{
  emit onRemoveStreamServer();
}

void StreamServerWidget::onToggleKey()
{
  // TODO
}