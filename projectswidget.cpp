#include "projectswidget.h"
#include <QMouseEvent>
#include "newprojectdialog.h"
#include "projectmenuitem.h"
#include "project_manager.h"

ProjectsWidget::ProjectsWidget(QWidget *_parent)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  ProjectManager& pm = ProjectManager::instance();
  auto projects = pm.listProjects();
  for(int i = 0; i < projects.size(); i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.listWidget);
    lwi->setSizeHint(QSize(0, 28));
    ProjectMenuItem* pmi = new ProjectMenuItem(projects[i]);
    ui.listWidget->addItem(lwi);
    ui.listWidget->setItemWidget(lwi, pmi);
  }

  updateSizeFromList();

  qApp->installEventFilter(this);
}

ProjectsWidget::~ProjectsWidget()
{

}

static int listWidgetHeightForItems(QListWidget* list)
{
  int h = 0;

  for(int i = 0; i < list->count(); ++i)
    h += list->sizeHintForRow(i);

  // frame + spacing
  h += 2 * list->frameWidth();
  h += list->spacing() * (list->count() - 1);

  return qMax(h, 50);
}

void ProjectsWidget::updateSizeFromList()
{
  int listHeight = listWidgetHeightForItems(ui.listWidget);

  ui.listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui.listWidget->setFixedHeight(listHeight);

  adjustSize();
}

bool ProjectsWidget::eventFilter(QObject* obj, QEvent* event)
{
  if(event->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(event);

    if(isVisible() && !this->geometry().contains(me->globalPosition().toPoint()))
    {
      hide();
      return true;
    }
  }
  return QDialog::eventFilter(obj, event);
}

void ProjectsWidget::onNewProject()
{
  hide();
  
  NewProjectDialog dlg(this);
  dlg.setWindowModality(Qt::ApplicationModal);

  // center
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if(!screen) screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();
  // dlg.adjustSize();
  dlg.move(screenGeometry.center() - dlg.rect().center());

  dlg.exec();
}