#include "projectmenuitem.h"
#include "project_manager.h"

ProjectMenuItem::ProjectMenuItem(Project &_project, QWidget *_parent)
:QWidget(_parent)
,project_(_project)
{
  ui.setupUi(this);
  onEndEdit();

  ProjectManager& pm = ProjectManager::instance();
  connect(&pm, &ProjectManager::projectListChanged, this, &ProjectMenuItem::onProjectListChanged);
  onProjectListChanged();
}

ProjectMenuItem::~ProjectMenuItem()
{

}

void ProjectMenuItem::onStartEdit()
{
  ui.stackedWidget->setCurrentIndex(1);
}

void ProjectMenuItem::onEndEdit()
{
  ui.stackedWidget->setCurrentIndex(0);
  if(project_)
  {
    ui.nameLabel->setText(project_->name);
    ui.lineEdit->setText(project_->name);
  }
}

void ProjectMenuItem::onProjectListChanged()
{
  ProjectManager& pm = ProjectManager::instance();
  auto projects = pm.listProjects();
  ui.deleteButton->setVisible(projects.size() > 1);
}

void ProjectMenuItem::onValidateEdit()
{
  QString newName = ui.lineEdit->text();
  if(!newName.isEmpty())
  {
    if(project_)
    {
      project_->name = newName;
      ProjectManager& pm = ProjectManager::instance();
      if(!pm.saveProject(*project_))
      {
        newName = ui.nameLabel->text();
      }
    }
    ui.stackedWidget->setCurrentIndex(0);
    ui.nameLabel->setText(newName);
    ui.lineEdit->setText(newName);
  }
}
