#pragma once

#include <QWidget>
#include "ui_livesourcewidget.h"
#include "basesourcewidget.h"

class LiveSourceWidget : public BaseSourceWidget
{
Q_OBJECT

public:
  LiveSourceWidget(QWidget *parent = nullptr);
  ~LiveSourceWidget();

  void init() override;
  SourceType type() override { return SourceType::EST_LIVE_SOURCE; }
  int h() override { return 125; }
  Source source() override;
  bool isValid() override;
  void editSource(const Source& _source) override;

protected slots:
  void fillLiveCombo();

private:
    Ui::LiveSourceWidgetClass ui;
};

