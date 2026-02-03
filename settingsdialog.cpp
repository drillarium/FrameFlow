#include "settingsdialog.h"
#include <QKeyEvent>
#include "streamserverwidget.h"

SettingsDialog::SettingsDialog(QWidget *_parent)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  ui.streamServersListWidgets->setFixedHeight(0);
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Escape)
  {
    event->ignore();
    return;
  }
  QDialog::keyPressEvent(event);
}

void SettingsDialog::onAddStreamServer()
{
  QListWidgetItem* lwi = new QListWidgetItem(ui.streamServersListWidgets);
  lwi->setSizeHint(QSize(0, 205));
  StreamServerWidget* ssw = new StreamServerWidget();
  auto lw = ui.streamServersListWidgets;
  connect(ssw, &StreamServerWidget::onRemoveStreamServer, this, [lw, lwi] () {
    int row = lw->row(lwi);
    QListWidgetItem* it = lw->takeItem(row);
    delete it;
    lw->setFixedHeight((205 * lw->count()) + (6 * lw->count()));
  });
  ui.streamServersListWidgets->addItem(lwi);
  ui.streamServersListWidgets->setItemWidget(lwi, ssw);

  ui.streamServersListWidgets->setFixedHeight((205 * ui.streamServersListWidgets->count()) + (6 * ui.streamServersListWidgets->count()) );
}
