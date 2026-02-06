#include "colorpickerwidget.h"
#include <QColorDialog>

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
:BaseSourceWidget(parent)
{
  ui.setupUi(this);
}

ColorPickerWidget::~ColorPickerWidget()
{
}

Source ColorPickerWidget::source()
{
  QJsonObject jsonConfig;
  jsonConfig.insert("color", ui.colorLineEdit->text());

  Source source;
  source.name = ui.nameLineEdit->text();
  source.type = type();
  source.config = jsonConfig;
  
  return source;
}

bool ColorPickerWidget::isValid()
{
  return (ui.nameLineEdit->text().size() > 0);
}

void ColorPickerWidget::onPickColor()
{
  QColor initialColor(ui.colorLineEdit->text());
  QColorDialog dlg(this);
  dlg.setOption(QColorDialog::ShowAlphaChannel, true);
  dlg.setStyleSheet(R"(
    QColorDialog {
        background-color: #2b2b2b;
    }

    QLabel {
        color: white;
    }

    QPushButton {
        background-color: #444;
        color: white;
        border-radius: 4px;
        padding: 6px;
        min-width: 120px;
    }

    QPushButton:hover {
        background-color: #555;
    }

    QColorDialog QSpinBox,
    QColorDialog QDoubleSpinBox {
        color: white;
        background-color: #2b2b2b;
        border: 1px solid #555;
    }

    QColorDialog QSpinBox QLineEdit,
    QColorDialog QDoubleSpinBox QLineEdit {
        color: white;
        background: transparent;
        selection-background-color: #555;
    }

    QColorDialog QSpinBox::up-arrow,
    QColorDialog QSpinBox::down-arrow,
    QColorDialog QDoubleSpinBox::up-arrow,
    QColorDialog QDoubleSpinBox::down-arrow {
        image: none;
    }
  )");
  if(dlg.exec() == QDialog::Accepted)
  {
    QColor color = dlg.currentColor();
    if(color.isValid())
    {
      QString s = QString("#%1%2%3%4").arg(color.alpha(), 2, 16, QLatin1Char('0')).arg(color.red(), 2, 16, QLatin1Char('0')).arg(color.green(), 2, 16, QLatin1Char('0')).arg(color.blue(), 2, 16, QLatin1Char('0')).toLower();
      ui.colorLineEdit->setText(s);
      ui.colorPickerButton->setStyleSheet(QString("border: 1px solid #647081; border-radius: 4px; background-color: rgba(%1,%2,%3,%4);").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
    }
  }
}