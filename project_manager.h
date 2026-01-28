#pragma once

#include <QObject>
#include <QVector>
#include <optional>
#include "project_model.h"

class ProjectManager : public QObject
{
  Q_OBJECT

public:
  static ProjectManager& instance();

  bool openDatabase(const QString& dbPath);

  // Project lifecycle
  bool createProject(const QString& name);
  bool loadProject(const QUuid& projectId);
  bool saveCurrentProject();
  void closeProject();
  bool deleteProject(const QUuid& projectId);

  // Queries
  QVector<Project> listProjects() const;
  std::optional<Project> currentProject() const { return currentProject_; }

  bool isDirty() const;
  void setDirty(bool dirty = true);

signals:
  void currentProjectChanged();
  void projectListChanged();
  void dirtyChanged(bool dirty);

private:
  explicit ProjectManager(QObject* parent = nullptr);
  ~ProjectManager() = default;

  Q_DISABLE_COPY_MOVE(ProjectManager)

private:
  std::optional<Project> currentProject_;
  QVector<Project> cachedProjects_;
  bool dirty_ = false;
};
