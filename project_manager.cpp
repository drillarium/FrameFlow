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
  Scene scene = {};
  scene.id = QUuid::createUuid();
  scene.name = "Main Scene";
  scene.orderIndex = p.scenes.size();
  p.scenes.push_back(scene);
  return createProject(p);
}

bool ProjectManager::createProject(Project& project)
{
  project.id = QUuid::createUuid();
  project.createdAt = QDateTime::currentDateTimeUtc();
  project.modifiedAt = project.createdAt;
  for(int i = 0; i < project.scenes.size(); i++)
  {
    project.scenes[i].projectId = project.id;
  }

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

bool ProjectManager::addScene(Scene& _scene)
{
  if(!currentProject_) return false;
  
  _scene.id = QUuid::createUuid();
  _scene.projectId = currentProject_->id;
  currentProject_->scenes.push_back(_scene);

  saveCurrentProject();

  emit currentProjectChanged();

  return true;
}

bool ProjectManager::removeScene(const QUuid& sceneId)
{
  if(!currentProject_) return false;

  auto it = std::find_if(currentProject_->scenes.begin(), currentProject_->scenes.end(), [sceneId] (const Scene& s) { return s.id == sceneId; });
  if(it == currentProject_->scenes.end()) return false;
  currentProject_->scenes.erase(it);

  // normalize order index
  for(int i = 0; i < currentProject_->scenes.size(); ++i) currentProject_->scenes[i].orderIndex = i;

  saveCurrentProject();
  Database::instance().deleteScene(sceneId);

  emit currentProjectChanged();

  return true;
}

bool ProjectManager::renameScene(const QUuid& sceneId, const QString& _newName)
{
  if(!currentProject_) return false;

  auto it = std::find_if(currentProject_->scenes.begin(), currentProject_->scenes.end(), [sceneId](const Scene& s) { return s.id == sceneId; });
  if(it == currentProject_->scenes.end()) return false;
  it->name = _newName;

  saveCurrentProject();
  emit currentProjectChanged();

  return true;
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

bool ProjectManager::addSource(Source &_source)
{
  if(!currentProject_) return false;

  for(int i = 0; i < currentProject_->scenes.size(); ++i)
  {
    if(currentProject_->scenes[i].id == currentSceneId_)
    {
      _source.id = QUuid::createUuid();
      _source.orderIndex = currentProject_->scenes[i].sources.size();
      _source.sceneId = currentProject_->scenes[i].id;
      currentProject_->scenes[i].sources.push_back(_source);

      saveCurrentProject();

      emit currentProjectChanged();
      
      return true;
    }
  }

  return false;
}