#pragma once

#include <QWidget>
#include "ui_sourceitemwidget.h"
#include "sourcemanager.h"

class SourceItemWidget : public QWidget
{
Q_OBJECT

public:
  SourceItemWidget(ESourceType type, QWidget *parent = nullptr);
  ~SourceItemWidget();

  void setSelected(bool _selected);

private:
  ESourceType type_;
  Ui::SourceItemWidgetClass ui;
};

