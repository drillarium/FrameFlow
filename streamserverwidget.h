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

  QUuid id() { return streamingServer_.id; }
  StreamingServer server() { return streamingServer_; }

protected slots:
  void onSave();
  void onRemove();
  void onToggleKey();

signals:
  void onRemoveStreamServer();
  void onSaveStreamServer();

private:
  Ui::StreamServerWidgetClass ui;
  StreamingServer streamingServer_;
};

