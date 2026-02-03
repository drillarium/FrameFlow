#pragma once

#include <QWidget>
#include "ui_streamserverwidget.h"

class StreamServerWidget : public QWidget
{
Q_OBJECT

public:
  StreamServerWidget(QWidget *parent = nullptr);
  ~StreamServerWidget();

protected slots:
  void onRemoveButton();

signals:
  void onRemoveStreamServer();

private:
  Ui::StreamServerWidgetClass ui;
};

