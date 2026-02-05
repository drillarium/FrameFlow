#include "baserenderer.h"

M_VID_PROPS BaseRenderer::getMVideoFormat(int w, int h, double fr)
{
  if(w == 1920 && h == 1080 && fr == 24) return { eMVideoFormat::eMVF_HD1080_24p };
  if(w == 1920 && h == 1080 && fr == 25) return { eMVideoFormat::eMVF_HD1080_25p };
  if(w == 1920 && h == 1080 && fr == 30) return { eMVideoFormat::eMVF_HD1080_30p };
  if(w == 1920 && h == 1080 && fr == 50) return { eMVideoFormat::eMVF_HD1080_50p };
  if(w == 1920 && h == 1080 && fr == 60) return { eMVideoFormat::eMVF_HD1080_60p };
  if(w == 1920 && h == 1080 && fr == 120) return { eMVideoFormat::eMVF_HD1080_120p };

  if(w == 2560 && h == 1440 && fr == 24) return { eMVideoFormat::eMVF_2K_24p };
  if(w == 2560 && h == 1440 && fr == 25) return { eMVideoFormat::eMVF_2K_25p };
  // if(w == 2560 && h == 1440 && fr == 30) return { eMVideoFormat::eMVF_2K_30p };
  // if(w == 2560 && h == 1440 && fr == 50) return { eMVideoFormat::eMVF_2K_50p };
  // if(w == 2560 && h == 1440 && fr == 60) return { eMVideoFormat::eMVF_2K_60p };
  // if(w == 2560 && h == 1440 && fr == 120) return { eMVideoFormat::eMVF_2K_120p };

  if(w == 3840 && h == 2160 && fr == 24) return { eMVideoFormat::eMVF_4K_UHD_24p };
  if(w == 3840 && h == 2160 && fr == 25) return { eMVideoFormat::eMVF_4K_UHD_25p };
  if(w == 3840 && h == 2160 && fr == 30) return { eMVideoFormat::eMVF_4K_UHD_30p };
  if(w == 3840 && h == 2160 && fr == 50) return { eMVideoFormat::eMVF_4K_UHD_50p };
  if(w == 3840 && h == 2160 && fr == 60) return { eMVideoFormat::eMVF_4K_UHD_60p };
  if(w == 3840 && h == 2160 && fr == 120) return { eMVideoFormat::eMVF_4K_UHD_120p };

  // if(w == 1280 && h == 720 && fr == 24) return { eMVideoFormat::eMVF_HD720_24p };
  // if(w == 1280 && h == 720 && fr == 25) return { eMVideoFormat::eMVF_HD720_25p };
  // if(w == 1280 && h == 720 && fr == 30) return { eMVideoFormat::eMVF_HD720_30p };
  if(w == 1280 && h == 720 && fr == 50) return { eMVideoFormat::eMVF_HD720_50p };
  if(w == 1280 && h == 720 && fr == 60) return { eMVideoFormat::eMVF_HD720_60p };
  // if(w == 1280 && h == 720 && fr == 120) return { eMVideoFormat::eMVF_HD720_120p };

  // custom
  M_VID_PROPS vProps = { eMVideoFormat::eMVF_Custom };
  vProps.nWidth = w;
  vProps.nHeight = h;
  vProps.dblRate = fr;
  vProps.eInterlace = eMInterlace::eMI_Progressive;

  return vProps;
}

void BaseRenderer::getFactors(double fr, int& num, int& den)
{
  if(fr == 24) { num = 24; den = 1; }
  if(fr == 25) { num = 25; den = 1; }
  if(fr == 30) { num = 30; den = 1; }
  if(fr == 50) { num = 50; den = 1; }
  if(fr == 60) { num = 60; den = 1; }
  if(fr == 120) { num = 120; den = 1; }
}