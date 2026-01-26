#pragma once

#include <QWidget>
#include "ui_sourcewidget.h"

class SourceWidget : public QWidget
{
Q_OBJECT

public:
  SourceWidget(QWidget *_parent = nullptr);
  ~SourceWidget();
  void setSelected(bool _selected);

protected:
  void enterEvent(QEnterEvent*) override;
  void leaveEvent(QEvent*) override;

private:
  Ui::SourceWidgetClass ui;
};

