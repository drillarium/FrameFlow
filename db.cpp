#include "db.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QDebug>

static constexpr int CURRENT_SCHEMA_VERSION = 2;

Database& Database::instance()
{
  static Database instance;
  return instance;
}

Database::Database()
{
}

Database::~Database()
{
  close();
}

bool Database::open(const QString& filePath)
{
  QMutexLocker lock(&mutex_);

  if(db_.isOpen()) return true;

  db_ = QSqlDatabase::addDatabase("QSQLITE");
  db_.setDatabaseName(filePath);

  if(!db_.open())
  {
    qCritical() << "DB open failed:" << db_.lastError();
    return false;
  }

  QSqlQuery q;
  q.exec("PRAGMA foreign_keys = ON;");

  return createSchemaIfNeeded();
}

void Database::close()
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return;

  db_.close();
  QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

bool Database::isOpen() const
{
  return db_.isOpen();
}
bool Database::createSchemaIfNeeded()
{
  QSqlQuery q(db_);

  // Does meta table exist?
  if(!q.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='meta'"))
  {
    qCritical() << q.lastError();
    return false;
  }

  if(!q.next())
  {
    // Fresh DB => create schema v1
    if(!execSchemaStatements(schemaV1Statements())) return false;

    if(!setSchemaVersion(1)) return false;
  }

  int version = schemaVersion();
  if(version < 0) return false;

  // Incremental migrations
  while(version < CURRENT_SCHEMA_VERSION)
  {
    switch(version)
    {
      case 1:
        if(!migrateV1ToV2())  return false;
        version = 2;
      break;
      default:
        qCritical() << "Unknown schema version:" << version;
      return false;
    }
  }

  return true;
}

bool Database::migrateV1ToV2()
{
  qDebug() << "Migrating schema v1 => v2";

  db_.transaction();

  QSqlQuery q(db_);

  const QStringList stmts = {
      "ALTER TABLE projects ADD COLUMN width INTEGER NOT NULL DEFAULT 1920",
      "ALTER TABLE projects ADD COLUMN height INTEGER NOT NULL DEFAULT 1080",
      "ALTER TABLE projects ADD COLUMN framerate REAL NOT NULL DEFAULT 30.0"
  };

  for(const QString& stmt : stmts)
  {
    if(!q.exec(stmt))
    {
      qCritical() << "Migration v1=>v2 failed:" << q.lastError();
      db_.rollback();
      return false;
    }
  }

  if(!setSchemaVersion(2))
  {
    db_.rollback();
    return false;
  }

  return db_.commit();
}

QStringList Database::schemaV1Statements() const
{
  return {

          R"(CREATE TABLE meta (
            key TEXT PRIMARY KEY,
            value TEXT
        ))",

        R"(INSERT INTO meta(key, value) VALUES ('schema_version', '1'))",

        R"(CREATE TABLE projects (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            created_at TEXT NOT NULL,
            modified_at TEXT NOT NULL
        ))",

        R"(CREATE TABLE scenes (
            id TEXT PRIMARY KEY,
            project_id TEXT NOT NULL,
            name TEXT NOT NULL,
            order_index INTEGER NOT NULL,
            settings TEXT,
            FOREIGN KEY(project_id) REFERENCES projects(id) ON DELETE CASCADE
        ))",

        R"(CREATE TABLE sources (
            id TEXT PRIMARY KEY,
            scene_id TEXT NOT NULL,
            type INTEGER NOT NULL,
            name TEXT NOT NULL,
            order_index INTEGER NOT NULL,
            config TEXT,
            FOREIGN KEY(scene_id) REFERENCES scenes(id) ON DELETE CASCADE
        ))"
  };
}

bool Database::execSchemaStatements(const QStringList& statements)
{
  db_.transaction();

  QSqlQuery q;
  for(const QString& stmt : statements)
  {
    if(!q.exec(stmt))
    {
      qCritical() << "Schema error:" << q.lastError();
      db_.rollback();
      return false;
    }
  }

  return db_.commit();
}

bool Database::saveProject(const Project& project)
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return false;

  db_.transaction();

  QSqlQuery q;
  q.prepare(R"(
        INSERT INTO projects(id, name, description, width, height, framerate, created_at, modified_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
            name=excluded.name,
            description=excluded.description,
            width=excluded.width,
            height=excluded.height,
            framerate=excluded.framerate,
            modified_at=excluded.modified_at
    )");

  q.addBindValue(project.id.toString(QUuid::WithoutBraces));
  q.addBindValue(project.name);
  q.addBindValue(project.description);
  q.addBindValue(project.width);
  q.addBindValue(project.height);
  q.addBindValue(project.framerate);
  q.addBindValue(project.createdAt.toUTC().toString(Qt::ISODate));
  q.addBindValue(project.modifiedAt.toUTC().toString(Qt::ISODate));

  if(!q.exec()) goto fail;

  for(const Scene& scene : project.scenes)
  {

    QSqlQuery qs;
    qs.prepare(R"(
            INSERT INTO scenes(id, project_id, name, order_index, settings)
            VALUES (?, ?, ?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
                name=excluded.name,
                order_index=excluded.order_index,
                settings=excluded.settings
        )");

    qs.addBindValue(scene.id.toString(QUuid::WithoutBraces));
    qs.addBindValue(project.id.toString(QUuid::WithoutBraces));
    qs.addBindValue(scene.name);
    qs.addBindValue(scene.orderIndex);
    qs.addBindValue(QJsonDocument(scene.settings).toJson(QJsonDocument::Compact));

    if(!qs.exec()) goto fail;

    for(const Source& src : scene.sources) {

      QSqlQuery qsrc;
      qsrc.prepare(R"(
                INSERT INTO sources(id, scene_id, type, name, order_index, config)
                VALUES (?, ?, ?, ?, ?, ?)
                ON CONFLICT(id) DO UPDATE SET
                    type=excluded.type,
                    name=excluded.name,
                    order_index=excluded.order_index,
                    config=excluded.config
            )");

      qsrc.addBindValue(src.id.toString(QUuid::WithoutBraces));
      qsrc.addBindValue(scene.id.toString(QUuid::WithoutBraces));
      qsrc.addBindValue((int) src.type);
      qsrc.addBindValue(src.name);
      qsrc.addBindValue(src.orderIndex);
      qsrc.addBindValue(QJsonDocument(src.config).toJson(QJsonDocument::Compact));

      if(!qsrc.exec()) goto fail;
    }
  }

  return db_.commit();

fail:
  qCritical() << "Save failed:" << q.lastError();
  db_.rollback();
  return false;
}

std::optional<Project> Database::loadProject(const QUuid& projectId)
{
  QMutexLocker lock(&mutex_);

  QSqlQuery q;
  q.prepare("SELECT * FROM projects WHERE id=?");
  q.addBindValue(projectId.toString(QUuid::WithoutBraces));

  if(!q.exec() || !q.next()) return std::nullopt;

  Project p;
  p.id = projectId;
  p.name = q.value("name").toString();
  p.description = q.value("description").toString();
  p.width = q.value("width").toInt();
  p.height = q.value("height").toInt();
  p.framerate = q.value("framerate").toDouble();
  p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
  p.modifiedAt = QDateTime::fromString(q.value("modified_at").toString(), Qt::ISODate);

  QSqlQuery qs;
  qs.prepare("SELECT * FROM scenes WHERE project_id=? ORDER BY order_index");
  qs.addBindValue(projectId.toString(QUuid::WithoutBraces));
  qs.exec();

  while(qs.next())
  {
    Scene s;
    s.id = QUuid(qs.value("id").toString());
    s.projectId = projectId;
    s.name = qs.value("name").toString();
    s.orderIndex = qs.value("order_index").toInt();
    s.settings = QJsonDocument::fromJson(
      qs.value("settings").toByteArray()
    ).object();

    QSqlQuery qsrc;
    qsrc.prepare("SELECT * FROM sources WHERE scene_id=? ORDER BY order_index");
    qsrc.addBindValue(s.id.toString(QUuid::WithoutBraces));
    qsrc.exec();

    while(qsrc.next())
    {
      Source src;
      src.id = QUuid(qsrc.value("id").toString());
      src.sceneId = s.id;
      src.type = (SourceType) qsrc.value("type").toInt();
      src.name = qsrc.value("name").toString();
      src.orderIndex = qsrc.value("order_index").toInt();
      src.config = QJsonDocument::fromJson(qsrc.value("config").toByteArray()).object();

      s.sources.push_back(src);
    }

    p.scenes.push_back(s);

    // sort by order index
    std::sort(p.scenes.begin(), p.scenes.end(), [](const Scene& a, const Scene& b) { return a.orderIndex < b.orderIndex; });
  }

  return p;
}

bool Database::deleteProject(const QUuid& projectId)
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return false;

  db_.transaction();

  QSqlQuery q;
  q.prepare("DELETE FROM projects WHERE id=?");
  q.addBindValue(projectId.toString(QUuid::WithoutBraces));

  if(!q.exec())
  {
    qCritical() << "Delete project failed:" << q.lastError();
    db_.rollback();
    return false;
  }

  // Optional: check if something was actually deleted
  if(q.numRowsAffected() == 0)
  {
    qWarning() << "No project deleted (id not found)";
    db_.rollback();
    return false;
  }

  return db_.commit();
}

int Database::schemaVersion() const
{
  if(!db_.isOpen())
    return -1;

  QSqlQuery q(db_);
  q.prepare("SELECT value FROM meta WHERE key='schema_version'");

  if(!q.exec())
  {
    qCritical() << "Failed to read schema_version:" << q.lastError();
    return -1;
  }

  if(!q.next()) return 0; // meta exists but no version yet

  return q.value(0).toInt();
}

bool Database::setSchemaVersion(int version)
{
  if(!db_.isOpen()) return false;

  QSqlQuery q(db_);
  q.prepare(R"(
        INSERT INTO meta(key, value)
        VALUES ('schema_version', ?)
        ON CONFLICT(key) DO UPDATE SET
            value=excluded.value
    )");

  q.addBindValue(QString::number(version));

  if(!q.exec())
  {
    qCritical() << "Failed to set schema_version:" << q.lastError();
    return false;
  }

  return true;
}

QVector<Project> Database::listProjects()
{
  QVector<Project> projects;

  QSqlQuery q("SELECT * FROM projects ORDER BY modified_at DESC");

  while(q.next())
  {
    Project p;
    p.id = QUuid(q.value("id").toString());
    p.name = q.value("name").toString();
    p.description = q.value("description").toString();
    p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    p.modifiedAt = QDateTime::fromString(q.value("modified_at").toString(), Qt::ISODate);

    projects.push_back(p);
  }

  return projects;
}

bool Database::deleteScene(const QUuid& sceneId)
{
  QSqlQuery q;
  q.prepare("DELETE FROM scenes WHERE id = ?");
  q.addBindValue(sceneId.toString(QUuid::WithoutBraces));

  if(!q.exec())
  {
    qWarning() << "Delete scene failed:" << q.lastError();
    return false;
  }
  return true;
}

bool Database::deleteSource(const QUuid& sourceId)
{
  QSqlQuery q;
  q.prepare("DELETE FROM sources WHERE id = ?");
  q.addBindValue(sourceId.toString(QUuid::WithoutBraces));

  if(!q.exec())
  {
    qWarning() << "Delete source failed:" << q.lastError();
    return false;
  }
  return true;
}
