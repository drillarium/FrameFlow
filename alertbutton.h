#pragma once

#include <QPushButton>
#include "notification_model.h"

class AlertButton  : public QPushButton
{
    Q_OBJECT

public:
  AlertButton(QWidget *parent);
  ~AlertButton();

  void setHasAlert(bool value, ENotificationSeverity _severity = ENotificationSeverity::S_INFO);

protected:
  void paintEvent(QPaintEvent* e) override;

protected:
  bool hasAlert_ = false;
  ENotificationSeverity severity_ = ENotificationSeverity::S_INFO;
};

