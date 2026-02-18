#pragma once

#include <QUuid>

enum class TransitionType
{
  TT_CUT,
  TT_FADE,
  TT_SLIDE,
  EST_LAST
};

struct Transition
{
  QUuid id;
  TransitionType type = TransitionType::TT_CUT;
  QString name;
  int msDuration = 200;
};

static QString defaultNameForTransition(TransitionType type)
{
  switch(type)
  {
    case TransitionType::TT_CUT: return "Cut";
    case TransitionType::TT_FADE: return "Fade";
    case TransitionType::TT_SLIDE: return "Slide";
    default: break;
  }

  return "";
}