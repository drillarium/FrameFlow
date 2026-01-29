#include "projectmenuitem.h"
#include "project_manager.h"
#include "confirmationdialog.h"

bool ProjectMenuItem::deletingItem_ = false;

ProjectMenuItem::ProjectMenuItem(Project &_project, QWidget *_parent)
:QWidget(_parent)
,project_(_project)
{
  ui.setupUi(this);
  onEndEdit();

  ProjectManager& pm = ProjectManager::instance();
  connect(&pm, &ProjectManager::projectListChanged, this, &ProjectMenuItem::onProjectListChanged);
  onProjectListChanged();

  ui.nameLabel->installEventFilter(this);
}

ProjectMenuItem::~ProjectMenuItem()
{

}

bool ProjectMenuItem::eventFilter(QObject* obj, QEvent* event)
{
  if(obj == ui.nameLabel && event->type() == QEvent::MouseButtonPress)
  {
    emit itemClicked();
    return true;
  }
  return QWidget::eventFilter(obj, event);
}

void ProjectMenuItem::onStartEdit()
{
  ui.stackedWidget->setCurrentIndex(1);
}

void ProjectMenuItem::onEndEdit()
{
  if(!project_) return;

  ui.stackedWidget->setCurrentIndex(0);
  ui.nameLabel->setText(project_->name);
  ui.lineEdit->setText(project_->name);
}

void ProjectMenuItem::onProjectListChanged()
{
  ProjectManager& pm = ProjectManager::instance();
  auto projects = pm.listProjects();
  ui.deleteButton->setVisible(projects.size() > 1);
}

void ProjectMenuItem::onValidateEdit()
{
  if(!project_) return;

  QString newName = ui.lineEdit->text();
  if(!newName.isEmpty())
  {
    project_->name = newName;
    ProjectManager& pm = ProjectManager::instance();
    if(!pm.saveProject(*project_))
    {
      newName = ui.nameLabel->text();
    }

    ui.stackedWidget->setCurrentIndex(0);
    ui.nameLabel->setText(newName);
    ui.lineEdit->setText(newName);
  }
}

void ProjectMenuItem::onDeleteProject()
{
  if(!project_) return;

  ProjectMenuItem::deletingItem_ = true;
  QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Remove project", "Are you sure you want to remove this project?", QMessageBox::Yes, QMessageBox::No, QMessageBox::No);
  if(reply == QMessageBox::Yes)
  {
    ProjectManager& pm = ProjectManager::instance();
    pm.deleteProject(project_->id);
  }
  ProjectMenuItem::deletingItem_ = false;
}

void ProjectMenuItem::setCurrentProject(QUuid _current)
{
  if(!project_) return;
  ui.dotLabel->setStyleSheet(QString("color: %1;").arg(project_->id == _current? "#19BDDE" : "black"));
}

