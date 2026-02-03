#include "streamserverwidget.h"

StreamServerWidget::StreamServerWidget(QWidget *parent)
:QWidget(parent)
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