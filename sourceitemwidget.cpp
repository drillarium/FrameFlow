#include "sourceitemwidget.h"
#include <QStyle>

QString sourceTitle(SourceType type)
{
  switch(type)
  {
    case SourceType::EST_COLOR: return "Color";
    case SourceType::EST_FILE: return "File";
    case SourceType::EST_URL: return "URL";
    case SourceType::EST_LIVE_SOURCE: return "Live Source";
    case SourceType::EST_NDI: return "NDI";
    case SourceType::EST_DEVICE: return "Device";
    case SourceType::EST_BROWSER: return "Browser";
    case SourceType::EST_SCREEN_CAPTURE: return "Screen Capture";
    case SourceType::EST_TEXT: return "Text";
    case SourceType::EST_SCENE: return "Scene";
    default: break;
  }

  return "";
}

QString sourceDescription(SourceType type)
{
  switch(type)
  {
  case SourceType::EST_COLOR: return "Generate a plain color frame";
  case SourceType::EST_FILE: return "Load a video or image file from disk";
  case SourceType::EST_URL: return "Stream video from a URL";
  case SourceType::EST_LIVE_SOURCE: return "Capture from a live video input";
  case SourceType::EST_NDI: return "Receive video from NDI network sources";
  case SourceType::EST_DEVICE: return "Capture from a connected device";
  case SourceType::EST_BROWSER: return "Render a web page as a video source";
  case SourceType::EST_SCREEN_CAPTURE: return "Capture the screen or a window";
  case SourceType::EST_TEXT: return "Render text as a video source";
  case SourceType::EST_SCENE: return "Exisitng scene as video source";
    default: break;
  }

  return "";
}

SourceItemWidget::SourceItemWidget(SourceType type, QWidget *parent)
:QWidget(parent)
,type_(type)
{
  ui.setupUi(this);
  ui.titleLabel->setText(sourceTitle(type));
  ui.descLabel->setText(sourceDescription(type));
}

SourceItemWidget::~SourceItemWidget()
{

}

void SourceItemWidget::setSelected(bool _selected)
{
  QString ss = "#mainSourceItemWidget {\
  background: #171B22;\
  border: 1px solid %1;\
  border-radius: 4px;\
}\
\
#mainSourceItemWidget:hover {\
  background: #303541;\
  border: 1px solid %2;\
  border-radius: 4px;\
}\
\
#titleLabel {\
  color: white;\
}\
\
#descLabel {\
  color: #7B899D;\
}";

  QString color = _selected ? "#19BDDE" : "transparent";
  QString color2 = _selected ? "#19BDDE" : "#7B899D";
  setStyleSheet(QString(ss).arg(color).arg(color2));
  style()->unpolish(this);
  style()->polish(this);
  update();
}
