#include "frameflow.h"
#include <QtWidgets/QApplication>
#include "splashwidget.h"
#include <QThread>
#include <QStandardPaths>
#include <QDir>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QCoreApplication::setOrganizationName("AVIO");
  QCoreApplication::setApplicationName("FrameFlow");

  // writable folder
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(dataDir);
  if(!dir.exists()) dir.mkpath(".");
  dir.mkpath("logs");

#ifndef _DEBUG
  SplashWidget splash;
  splash.show();

  splash.setMessage("Initializing engines..", 1, 3);
  QThread::msleep(1000);

  splash.setMessage("Loading plugins...", 2, 3);
  QThread::msleep(1000);

  splash.setMessage("Starting UI...", 3, 3);
  QThread::msleep(1000);
#endif // _DEBUG

  FrameFlow window;
  window.show();

#ifndef _DEBUG
  splash.close();
#endif // _DEBUG

  return app.exec();
}
