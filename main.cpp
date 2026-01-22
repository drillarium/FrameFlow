#include "frameflow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  FrameFlow window;
  window.show();
  return app.exec();
}
