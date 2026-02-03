#include "alertsdialog.h"
#include <QKeyEvent>
#include "alertwidget.h"

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

  for(int i = 0; i < 10; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.listWidget);
    lwi->setSizeHint(QSize(0, 100));
    AlertWidget* aw = new AlertWidget();
    auto lw = ui.listWidget;
    connect(aw, &AlertWidget::onRemoveAlert, this, [lw, lwi]() {
      int row = lw->row(lwi);
      QListWidgetItem* it = lw->takeItem(row);
      delete it;
      lw->setFixedHeight((100 * lw->count()) + (6 * lw->count()));
      });
    ui.listWidget->addItem(lwi);
    ui.listWidget->setItemWidget(lwi, aw);

    ui.listWidget->setFixedHeight((100 * ui.listWidget->count()) + (6 * ui.listWidget->count()));
  }
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
  auto items = ui.listWidget->selectedItems();

  for(QListWidgetItem* item : items)
  {
    delete ui.listWidget->takeItem(ui.listWidget->row(item));
  }
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
