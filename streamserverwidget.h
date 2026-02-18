#pragma once

#include <QWidget>
#include "ui_streamserverwidget.h"
#include "server_model.h"

class StreamServerWidget : public QWidget
{
Q_OBJECT

public:
  StreamServerWidget(const StreamingServer &_ss, QWidget *parent = nullptr);
  ~StreamServerWidget();

protected slots:
  void onRemoveButton();

signals:
  void onRemoveStreamServer();

private:
  Ui::StreamServerWidgetClass ui;
  StreamingServer streamingServer_;
};

