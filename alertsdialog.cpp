#include "alertsdialog.h"
#include <QKeyEvent>
#include "alertwidget.h"
#include "notification_manager.h"

AlertsDialog::AlertsDialog(QWidget *_parent)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  auto modelSource = ui.listWidget->model();
  connect(modelSource, &QAbstractItemModel::rowsInserted, this, [this]() { checkNumNotifications(); });
  connect(modelSource, &QAbstractItemModel::rowsRemoved, this, [this]() { checkNumNotifications(); });
  connect(modelSource, &QAbstractItemModel::modelReset, this, [this]() { checkNumNotifications(); });

  // notifications
  NotificationManager &nm = NotificationManager::instance();
  connect(&nm, &NotificationManager::onNotificationAdded, this, &AlertsDialog::updateNotifications);
  connect(&nm, &NotificationManager::onNotificationUpdated, this, &AlertsDialog::updateNotifications);
  connect(&nm, &NotificationManager::onNotificationRemoved, this, &AlertsDialog::updateNotifications);
  updateNotifications();
}

AlertsDialog::~AlertsDialog()
{
  if(auto m = ui.listWidget->model()) { disconnect(m, nullptr, this, nullptr); }
}

void AlertsDialog::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Escape)
  {
    event->ignore();
    return;
  }
  QDialog::keyPressEvent(event);
}

void AlertsDialog::checkNumNotifications()
{
  if(ui.listWidget->count() == 0) ui.noAlertsWidget->show();
  else ui.noAlertsWidget->hide();
}

void AlertsDialog::onClearAll()
{
  clear_ = true;

  auto items = ui.listWidget->selectedItems();
  NotificationManager& nm = NotificationManager::instance();
  for(QListWidgetItem* item : items)
  {
    AlertWidget* w = static_cast<AlertWidget*>(ui.listWidget->itemWidget(item));
    if(w)
    {
      nm.unregisterNotification(w->id());
    }
  }

  clear_ = false;

  updateNotifications();
}

void AlertsDialog::onMarkAll()
{
  ui.listWidget->selectAll();
}

void AlertsDialog::onNotificationsSelectionChange()
{
  for(int i = 0; i < ui.listWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.listWidget->item(i);
    AlertWidget* w = static_cast<AlertWidget*>(ui.listWidget->itemWidget(item));
    if(w) w->setSelected(item->isSelected());
  }
}

void AlertsDialog::updateNotifications()
{
  if(clear_) return;

  NotificationManager& nm = NotificationManager::instance();
  auto notifications = nm.notificationList();
  int newItemCount = notifications.size();

  // update existing ones
  for(int i = 0; i < std::min(newItemCount, ui.listWidget->count()); ++i)
  {
    QListWidgetItem *lwi = ui.listWidget->item(i);
    AlertWidget *alertWidget = static_cast<AlertWidget*>(ui.listWidget->itemWidget(lwi));
    alertWidget->update(notifications[i]);
  }

  // crete new ones
  for(int i = ui.listWidget->count(); i < newItemCount; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.listWidget);
    lwi->setSizeHint(QSize(0, 100));
    AlertWidget* aw = new AlertWidget();
    aw->update(notifications[i]);
    QUuid id = notifications[i].id;
    connect(aw, &AlertWidget::onRemoveAlert, this, [id]() {
      NotificationManager& nm = NotificationManager::instance();
      nm.unregisterNotification(id);
    });
    ui.listWidget->addItem(lwi);
    ui.listWidget->setItemWidget(lwi, aw);
  }

  // remove unused
  int itemCount = ui.listWidget->count();
  for(int i = newItemCount; i < itemCount; i++)
  {
    QListWidgetItem* it = ui.listWidget->takeItem(newItemCount);
    delete it;
  }

  // list size
  ui.listWidget->setFixedHeight((100 * ui.listWidget->count()) + (6 * ui.listWidget->count()));
}
