#pragma once

#include <QWidget>
#include "ui_alertwidget.h"
#include "notification_model.h"

class AlertWidget : public QWidget
{
Q_OBJECT

public:
  AlertWidget(QWidget *parent = nullptr);
  ~AlertWidget();

  void setSelected(bool _selected);
  void update(const Notification &_notification);
  QUuid id() { return id_; }

protected:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;

signals:
  void onRemoveAlert();

private:
  Ui::AlertWidgetClass ui;
  QUuid id_;
};

