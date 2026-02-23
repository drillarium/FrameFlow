#pragma once

#include <QWidget>
#include "ui_sourceitemwidget.h"
#include "source_model.h"

class SourceItemWidget : public QWidget
{
Q_OBJECT

public:
  SourceItemWidget(SourceType type, QWidget *parent = nullptr);
  ~SourceItemWidget();

  void setSelected(bool _selected);

private:
  SourceType type_;
  Ui::SourceItemWidgetClass ui;
};

static QString sourceTitle(SourceType type)
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

static QString sourceDescription(SourceType type)
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
