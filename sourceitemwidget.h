#pragma once

#include <QWidget>
#include "ui_sourceitemwidget.h"
#include "source_model.h"

class SourceItemWidget : public QWidget
{
Q_OBJECT

public:
  SourceItemWidget(SourceType type, QWidget *parent = nullptr);
  ~SourceItemWidget();

  void setSelected(bool _selected);

private:
  SourceType type_;
  Ui::SourceItemWidgetClass ui;
};

