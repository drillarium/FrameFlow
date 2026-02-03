#include "sourceitemwidget.h"
#include <QStyle>

QString sourceTitle(ESourceType type)
{
  switch(type)
  {
    case ESourceType::EST_COLOR: return "Color";
    case ESourceType::EST_FILE: return "File";
    case ESourceType::EST_URL: return "URL";
    case ESourceType::EST_LIVE_SOURCE: return "Live Source";
    case ESourceType::EST_NDI: return "NDI";
    case ESourceType::EST_DEVICE: return "Device";
    case ESourceType::EST_WEBCAM: return "Webcam";
    case ESourceType::EST_BROWSER: return "Browser";
    case ESourceType::EST_SCREEN_CAPTURE: return "Screen Capture";
    case ESourceType::EST_TEXT: return "Text";
    default: break;
  }

  return "";
}

QString sourceDescription(ESourceType type)
{
  switch(type)
  {
  case ESourceType::EST_COLOR: return "Generate a plain color frame";
  case ESourceType::EST_FILE: return "Load a video or image file from disk";
  case ESourceType::EST_URL: return "Stream video from a URL";
  case ESourceType::EST_LIVE_SOURCE: return "Capture from a live video input";
  case ESourceType::EST_NDI: return "Receive video from NDI network sources";
  case ESourceType::EST_DEVICE: return "Capture from a connected device";
  case ESourceType::EST_WEBCAM: return "Capture from the local webcam";
  case ESourceType::EST_BROWSER: return "Render a web page as a video source";
  case ESourceType::EST_SCREEN_CAPTURE: return "Capture the screen or a window";
  case ESourceType::EST_TEXT: return "Render text as a video source";
    default: break;
  }

  return "";
}

SourceItemWidget::SourceItemWidget(ESourceType type, QWidget *parent)
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
