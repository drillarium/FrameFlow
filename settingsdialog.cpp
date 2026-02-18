#include "settingsdialog.h"
#include <QKeyEvent>
#include "streamserverwidget.h"
#include "project_manager.h"

SettingsDialog::SettingsDialog(QWidget *_parent)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  ui.streamServersListWidgets->setFixedHeight(0);

  ProjectManager &pm = ProjectManager::instance();
  auto ss = pm.listStreamingServers();

  for(StreamingServer s : ss)
  {
    addStreamingServer(s);
  }
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
  StreamingServer ss;
  ss.name = "New Streaming Server";
  ss.platform = "Custom RTMP";
  ss.enabled = true;

  ProjectManager& pm = ProjectManager::instance();
  if(pm.addStreamingServer(ss))
  {
    addStreamingServer(ss);
  }
}

void SettingsDialog::addStreamingServer(const StreamingServer &_ss)
{
  QListWidgetItem* lwi = new QListWidgetItem(ui.streamServersListWidgets);
  lwi->setSizeHint(QSize(0, 205));
  StreamServerWidget* ssw = new StreamServerWidget(_ss);
  auto lw = ui.streamServersListWidgets;
  connect(ssw, &StreamServerWidget::onRemoveStreamServer, this, [lw, lwi]() {
    int row = lw->row(lwi);
    QListWidgetItem* it = lw->takeItem(row);
    delete it;
    lw->setFixedHeight((205 * lw->count()) + (6 * lw->count()));
    });
  ui.streamServersListWidgets->addItem(lwi);
  ui.streamServersListWidgets->setItemWidget(lwi, ssw);

  ui.streamServersListWidgets->setFixedHeight((205 * ui.streamServersListWidgets->count()) + (6 * ui.streamServersListWidgets->count()));
}