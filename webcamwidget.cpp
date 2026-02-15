#include "webcamwidget.h"
#include "sourcedevicebaserenderer.h"

WebcamWidget::WebcamWidget(QWidget *parent)
:BaseSourceWidget(parent)
{
  ui.setupUi(this); 
}

WebcamWidget::~WebcamWidget()
{

}

void WebcamWidget::init()
{
  QStringList sl = SourceDeviceBaseRenderer::listOfDevices();
  ui.deviceComboBox->addItems(sl);
}

Source WebcamWidget::source()
{
  Source s = Source();
  Source& source = editing_ ? source_ : s;

  QJsonObject jsonConfig = editing_ ? source_.config : QJsonObject();
  jsonConfig.insert("device", ui.deviceComboBox->currentText());
  jsonConfig.insert("format", ui.formatComboBox->currentText());
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

bool WebcamWidget::isValid()
{
  return (ui.nameLineEdit->text().size() > 0);
}

void WebcamWidget::editSource(const Source& _source)
{
  BaseSourceWidget::editSource(_source);

  ui.nameLineEdit->setText(source_.name);
  if(source_.config.contains("device") && source_.config.value("device").isString())
  {
    QString device = source_.config["device"].toString();
    for(int i = 0; i < ui.deviceComboBox->count(); ++i)
    {
      if(ui.deviceComboBox->itemText(i) == device)
      {
        ui.deviceComboBox->setCurrentIndex(i);
        break;
      }
    }
  }

  if(source_.config.contains("format") && source_.config.value("format").isString())
  {
    QString format = source_.config["format"].toString();
    for(int i = 0; i < ui.formatComboBox->count(); ++i)
    {
      if(ui.formatComboBox->itemText(i) == format)
      {
        ui.formatComboBox->setCurrentIndex(i);
        break;
      }
    }
  }
}

void WebcamWidget::onDeviceSelectionChange()
{
  QString device = ui.deviceComboBox->currentText();
  QStringList sl = SourceDeviceBaseRenderer::deviceProps(device);
  ui.formatComboBox->clear();
  ui.formatComboBox->addItems(sl);
}
