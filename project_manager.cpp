#include "project_manager.h"
#include "db.h"

#include <QDateTime>

ProjectManager& ProjectManager::instance()
{
  static ProjectManager instance;
  return instance;
}

ProjectManager::ProjectManager(QObject* parent)
:QObject(parent)
{
}

bool ProjectManager::openDatabase(const QString& dbPath)
{
  if(!Database::instance().open(dbPath)) return false;

  cachedProjects_ = Database::instance().listProjects();
  emit projectListChanged();

  return true;
}

QVector<Project> ProjectManager::listProjects() const
{
  return cachedProjects_;
}

bool ProjectManager::createProject(const QString& name)
{
  Project p;
  p.name = name;
  return createProject(p);
}

bool ProjectManager::createProject(Project& project)
{
  project.id = QUuid::createUuid();
  project.createdAt = QDateTime::currentDateTimeUtc();
  project.modifiedAt = project.createdAt;

  if(!Database::instance().saveProject(project)) return false;

  currentProject_ = project;
  cachedProjects_ = Database::instance().listProjects();
  dirty_ = false;

  emit currentProjectChanged();
  emit projectListChanged();
  emit dirtyChanged(false);

  return true;
}

bool ProjectManager::loadProject(const QUuid& projectId)
{
  auto project = Database::instance().loadProject(projectId);
  if(!project) return false;

  currentProject_ = *project;
  dirty_ = false;

  emit currentProjectChanged();
  emit dirtyChanged(false);

  return true;
}

bool ProjectManager::saveProject(Project& _project)
{
  for(int i = 0; i < cachedProjects_.size(); i++)
  {
    if(cachedProjects_[i].id == _project.id)
    {
      _project.modifiedAt = QDateTime::currentDateTimeUtc();
      if(!Database::instance().saveProject(_project)) return false;

      dirty_ = false;
      cachedProjects_ = Database::instance().listProjects();

      // current project
      if(currentProject_ && currentProject_->id == _project.id)
      {
        currentProject_ = _project;
        emit currentProjectChanged();
        emit dirtyChanged(false);
      }
      emit projectListChanged();

      return true;
    }
  }

  return false;
}

bool ProjectManager::saveCurrentProject()
{
  if(!currentProject_) return false;

  currentProject_->modifiedAt = QDateTime::currentDateTimeUtc();

  if(!Database::instance().saveProject(*currentProject_)) return false;

  dirty_ = false;
  cachedProjects_ = Database::instance().listProjects();

  emit dirtyChanged(false);
  emit projectListChanged();

  return true;
}

bool ProjectManager::deleteProject(const QUuid& projectId)
{
  if(!Database::instance().deleteProject(projectId)) return false;

  bool isCurrent = (currentProject_ && currentProject_->id == projectId);
  if(isCurrent)
  {
    closeProject();
  }

  cachedProjects_ = Database::instance().listProjects();
  emit projectListChanged();

  // load first one
  if(isCurrent)
  {
    if(cachedProjects_.size() > 0)
    {
      loadProject(cachedProjects_[0].id);
    }
  }

  return true;
}

void ProjectManager::closeProject()
{
  if(!currentProject_) return;

  currentProject_.reset();
  dirty_ = false;

  emit currentProjectChanged();
  emit dirtyChanged(false);
}

bool ProjectManager::isDirty() const
{
  return dirty_;
}

void ProjectManager::setDirty(bool dirty)
{
  if(dirty_ == dirty) return;

  dirty_ = dirty;
  emit dirtyChanged(dirty_);
}

