#pragma once

#include <QWidget>
#include "ui_alertwidget.h"

class AlertWidget : public QWidget
{
Q_OBJECT

public:
  AlertWidget(QWidget *parent = nullptr);
  ~AlertWidget();

  void setSelected(bool _selected);

protected:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;

signals:
  void onRemoveAlert();

private:
    Ui::AlertWidgetClass ui;
};

