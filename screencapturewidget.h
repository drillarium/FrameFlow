#pragma once

#include <QWidget>
#include "ui_screencapturewidget.h"
#include "basesourcewidget.h"

class ScreenCaptureWidget : public BaseSourceWidget
{
Q_OBJECT

public:
  ScreenCaptureWidget(QWidget *parent = nullptr);
  ~ScreenCaptureWidget();

  void init() override;
  SourceType type() override { return SourceType::EST_SCREEN_CAPTURE; }
  int h() override { return 185; }
  Source source() override;
  bool isValid() override;
  void editSource(const Source& _source) override;

protected slots:
  void fillMonitorCombo();
  void onFillWindowHandlers();

private:
  Ui::ScreenCaptureWidgetClass ui;
};

