#include "streamserverwidget.h"

StreamServerWidget::StreamServerWidget(const StreamingServer& _ss, QWidget *parent)
:QWidget(parent)
,streamingServer_(_ss)
{
  ui.setupUi(this);

  ui.comboBox->setCurrentIndex(ui.comboBox->count() - 1);
}

StreamServerWidget::~StreamServerWidget()
{
}

void StreamServerWidget::onRemoveButton()
{
  // TODO: confirmation

  emit onRemoveStreamServer();
}