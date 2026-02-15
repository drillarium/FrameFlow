#include "screencapturewidget.h"
#include "sourcedevicebaserenderer.h"

ScreenCaptureWidget::ScreenCaptureWidget(QWidget *parent)
:BaseSourceWidget(parent)
{
  ui.setupUi(this);
}

ScreenCaptureWidget::~ScreenCaptureWidget()
{

}

void ScreenCaptureWidget::init()
{

  fillMonitorCombo();
  onFillWindowHandlers();
}

void ScreenCaptureWidget::fillMonitorCombo()
{
  ui.deviceMonitorComboBox->clear();

  QStringList sl = SourceDeviceBaseRenderer::listOfMonitors();
  ui.deviceMonitorComboBox->addItems(sl);
}

void ScreenCaptureWidget::onFillWindowHandlers()
{
  ui.handlersComboBox->clear();

  QStringList sl = QStringList() << "<None>";
  sl += SourceDeviceBaseRenderer::listOfWindowHandlers();
  ui.handlersComboBox->addItems(sl);
}

Source ScreenCaptureWidget::source()
{
  Source s = Source();
  Source& source = editing_ ? source_ : s;

  QJsonObject jsonConfig = editing_ ? source_.config : QJsonObject();
  jsonConfig.insert("monitor", ui.deviceMonitorComboBox->currentText());
  jsonConfig.insert("show_mouse", ui.showMouseButton->isChecked());
  jsonConfig.insert("window", ui.handlersComboBox->currentText());
  if(!editing_)
  {
    jsonConfig["x"] = 10;
    jsonConfig["y"] = 10;
    jsonConfig["width"] = 320;
    jsonConfig["height"] = 240;
  }

  // Source source;
  source.name = ui.nameLineEdit->text();
  source.type = type();
  source.config = jsonConfig;

  return source;
}

bool ScreenCaptureWidget::isValid()
{
  return (ui.nameLineEdit->text().size() > 0);
}

void ScreenCaptureWidget::editSource(const Source& _source)
{
  BaseSourceWidget::editSource(_source);

  ui.nameLineEdit->setText(source_.name);
  if(source_.config.contains("monitor") && source_.config.value("monitor").isString())
  {
    QString device = source_.config["monitor"].toString();
    ui.deviceMonitorComboBox->setCurrentText(device);
  }
  if(source_.config.contains("show_mouse") && source_.config.value("show_mouse").isBool())
  {
    bool show = source_.config["show_mouse"].toBool();
    ui.showMouseButton->setChecked(show);
  }
  if(source_.config.contains("window") && source_.config.value("window").isString())
  {
    QString window = source_.config["window"].toString();
    ui.handlersComboBox->setCurrentText(window);
  }
}
