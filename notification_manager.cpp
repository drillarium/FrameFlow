#include "notification_manager.h"
#include <QTimer>

NotificationManager::NotificationManager(QObject *parent)
:QObject(parent)
{
  // system state
  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &NotificationManager::checkNotifications);
  timer->start(60000);
}

NotificationManager& NotificationManager::instance()
{
  static NotificationManager instance;
  return instance;
}

void NotificationManager::checkNotifications()
{
  // TODO: Remove notifications depends on elapsed time since last modification
}

void NotificationManager::markAsRead(const QUuid &_id)
{
  for(Notification& n : notifications_)
  {
    if(n.id == _id)
    {
      n.read = true;
      n.modificationTime = QDateTime::currentDateTime();
      emit onNotificationUpdated(n);
      return;
    }
  }
}

void NotificationManager::removeRead()
{
  for(int i = notifications_.size() - 1; i >= 0; --i)
  {
    if(notifications_[i].read)
    {
      QUuid id = notifications_[i].id;
      notifications_.removeAt(i);
      emit onNotificationRemoved(id);
    }
  }
}

void NotificationManager::registerNotification(const Notification& _notification)
{
  Notification n = _notification;
  n.creationTime = QDateTime::currentDateTime();
  n.modificationTime = QDateTime::currentDateTime();
  notifications_.push_back(n);
  emit onNotificationAdded(n);
}

void NotificationManager::updateNotification(const Notification& _notification)
{
  for(Notification& n : notifications_)
  {
    if(n.id == _notification.id)
    {
      n = _notification;
      n.modificationTime = QDateTime::currentDateTime();
      emit onNotificationUpdated(n);
      return;
    }
  }
}

void NotificationManager::unregisterNotification(QUuid _id)
{
  for(int i = 0; i < notifications_.size(); ++i)
  {
    if(notifications_[i].id == _id)
    {
      notifications_.removeAt(i);
      emit onNotificationRemoved(_id);
      return;
    }
  }
}
