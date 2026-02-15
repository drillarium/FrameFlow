#include "ndiwidget.h"
#include "sourcedevicebaserenderer.h"

NDIWidget::NDIWidget(QWidget *parent)
:BaseSourceWidget(parent)
{
  ui.setupUi(this);
}

NDIWidget::~NDIWidget()
{

}

void NDIWidget::init()
{
  fillNDICombo();
  fillFormatCombo();
}

void NDIWidget::fillNDICombo()
{
  ui.deviceLineComboBox->clear();

  QStringList sl = SourceDeviceBaseRenderer::listOfNDILines();
  ui.deviceLineComboBox->addItems(sl);
}

void NDIWidget::fillFormatCombo()
{
  QStringList sl = SourceDeviceBaseRenderer::NDIDeviceProps();
  ui.formatComboBox->clear();
  ui.formatComboBox->addItems(sl);
}

Source NDIWidget::source()
{
  Source s = Source();
  Source& source = editing_ ? source_ : s;

  QJsonObject jsonConfig = editing_ ? source_.config : QJsonObject();
  jsonConfig.insert("line", ui.deviceLineComboBox->currentText());
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

bool NDIWidget::isValid()
{
  return (ui.nameLineEdit->text().size() > 0);
}

void NDIWidget::editSource(const Source& _source)
{
  BaseSourceWidget::editSource(_source);

  ui.nameLineEdit->setText(source_.name);
  if(source_.config.contains("line") && source_.config.value("line").isString())
  {
    QString device = source_.config["line"].toString();
    ui.deviceLineComboBox->setCurrentText(device);
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