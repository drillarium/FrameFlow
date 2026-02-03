#include "projectswidget.h"
#include <QMouseEvent>
#include "newprojectdialog.h"
#include "projectmenuitem.h"
#include "project_manager.h"
#include "confirmationdialog.h"

ProjectsWidget::ProjectsWidget(QWidget *_parent)
:QDialog(_parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  updateProjectList();

  qApp->installEventFilter(this);
}

ProjectsWidget::~ProjectsWidget()
{

}

void ProjectsWidget::updateProjectList()
{
  // clear
  ui.listWidget->setUpdatesEnabled(false);
  while(ui.listWidget->count() > 0)
  {
    QListWidgetItem* item = ui.listWidget->takeItem(0);
    QWidget* w = ui.listWidget->itemWidget(item);
    delete w;
    delete item;
  }
  ui.listWidget->setUpdatesEnabled(true);

  // populate
  ProjectManager& pm = ProjectManager::instance();
  auto currentProject = pm.currentProject();
  auto projects = pm.listProjects();
  for(int i = 0; i < projects.size(); i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.listWidget);
    lwi->setSizeHint(QSize(0, 28));
    ProjectMenuItem* pmi = new ProjectMenuItem(projects[i]);
    connect(pmi, &ProjectMenuItem::itemClicked, this, [=] () {
      QUuid id = pmi->id();
      switchToProject(id);
    });
    pmi->setCurrentProject(currentProject? currentProject->id : QUuid());
    ui.listWidget->addItem(lwi);
    ui.listWidget->setItemWidget(lwi, pmi);    
  }

  updateSizeFromList();
}

static int listWidgetHeightForItems(QListWidget* list)
{
  int h = 0;

  for(int i = 0; i < list->count(); ++i)
    h += 28; // list->sizeHintForRow(i);

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

  setFixedHeight(listHeight + 75);
  // adjustSize();
}

bool ProjectsWidget::eventFilter(QObject* obj, QEvent* event)
{
  if(event->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(event);
    if(isVisible() && !this->geometry().contains(me->globalPosition().toPoint()) && !ProjectMenuItem::deletingItem_ && !switchToProject_)
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

  if(dlg.exec() == QDialog::Accepted)
  {
    ProjectManager& pm = ProjectManager::instance();
    auto project = dlg.project();
    pm.createProject(project);
  }
}

void ProjectsWidget::setCurrentProject(QUuid uid)
{
  for(int i = 0; i < ui.listWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.listWidget->item(i);
    ProjectMenuItem *pmi = (ProjectMenuItem *) ui.listWidget->itemWidget(item);
    pmi->setCurrentProject(uid);
  }
}

void ProjectsWidget::switchToProject(QUuid& _project)
{
  // current project
  ProjectManager& pm = ProjectManager::instance();
  auto currentProject = pm.currentProject();
  if(currentProject && currentProject->id == _project) return;

  switchToProject_ = true;
  QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Load project", "Are you sure you want to load project?", QMessageBox::Yes, QMessageBox::No, QMessageBox::No);
  if(reply == QMessageBox::Yes)
  {
    hide();
    ProjectManager& pm = ProjectManager::instance();
    pm.loadProject(_project);
  }
  switchToProject_ = false;
}
