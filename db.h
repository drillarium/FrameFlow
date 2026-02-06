#pragma once

#include <QSqlDatabase>
#include <QMutex>
#include <optional>
#include "project_model.h"

class Database
{
public:
  static Database& instance();

  bool open(const QString& filePath);
  void close();
  bool isOpen() const;

  // Schema
  bool createSchemaIfNeeded();

  // Projects
  bool saveProject(const Project& project);
  std::optional<Project> loadProject(const QUuid& projectId);
  bool deleteProject(const QUuid& projectId);
  QVector<Project> listProjects();
  bool deleteScene(const QUuid& sceneId);
  bool deleteSource(const QUuid& sourceId);

private:
  Database();
  ~Database();

  Q_DISABLE_COPY_MOVE(Database)

  bool execSchemaStatements(const QStringList& statements);
  bool setSchemaVersion(int version);
  int  schemaVersion() const;
  QStringList schemaV1Statements() const;

  bool migrateV1ToV2();

private:
  QSqlDatabase db_;
  mutable QMutex mutex_;
};
