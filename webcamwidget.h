#pragma once

#include <QWidget>
#include "ui_webcamwidget.h"
#include "basesourcewidget.h"

class WebcamWidget : public BaseSourceWidget
{
Q_OBJECT

public:
  WebcamWidget(QWidget *parent = nullptr);
  ~WebcamWidget();

  void init() override;
  SourceType type() override { return SourceType::EST_DEVICE; }
  int h() override { return 180; }
  Source source() override;
  bool isValid() override;
  void editSource(const Source& _source) override;

protected slots:
  void onDeviceSelectionChange();

private:
  Ui::WebcamWidgetClass ui;
};

