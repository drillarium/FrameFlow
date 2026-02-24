#pragma once

#include <QObject>
#include "notification_model.h"

class NotificationManager  : public QObject
{
Q_OBJECT

public:
  static NotificationManager& instance();

  void registerNotification(const Notification &_notification);
  void updateNotification(const Notification& _notification);
  void unregisterNotification(QUuid _id);
  QList<Notification> notificationList() { return notifications_; }
  void markAsRead(const QUuid& _id);
  void removeRead();

signals:
  void onNotificationAdded(Notification _notification);
  void onNotificationUpdated(Notification _notification);
  void onNotificationRemoved(QUuid _id);

protected slots:
  void checkNotifications();

private:
  explicit NotificationManager(QObject* parent = nullptr);
  ~NotificationManager() = default;

  Q_DISABLE_COPY_MOVE(NotificationManager)

protected:
  QList<Notification> notifications_;
};

