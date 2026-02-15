#pragma once

#include <QWidget>
#include "ui_ndiwidget.h"
#include "basesourcewidget.h"

class NDIWidget : public BaseSourceWidget
{
Q_OBJECT

public:
  NDIWidget(QWidget *parent = nullptr);
  ~NDIWidget();

  void init() override;
  SourceType type() override { return SourceType::EST_NDI; }
  int h() override { return 180; }
  Source source() override;
  bool isValid() override;
  void editSource(const Source& _source) override;

protected slots:
  void fillNDICombo();
  void fillFormatCombo();

private:
    Ui::NDIWidgetClass ui;
};

