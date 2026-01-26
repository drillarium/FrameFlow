#include "frameflow.h"
#include <QtWidgets/QApplication>
#include "splashwidget.h"
#include <QThread>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

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
