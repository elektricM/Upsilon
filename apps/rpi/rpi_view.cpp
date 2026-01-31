#include "rpi_view.h"
#include <kandinsky/font.h>

namespace Rpi {

RpiView::RpiView() :
  View()
{
}

void RpiView::drawRect(KDContext * ctx, KDRect rect) const {
  constexpr KDColor bgColor = KDColor::RGB24(0xC51A4A);
  constexpr KDColor textColor = KDColorWhite;

  ctx->fillRect(bounds(), bgColor);

  const char * line1 = "Raspberry Pi";
  const char * line2 = "Not Connected";

  KDSize size1 = KDFont::LargeFont->stringSize(line1);
  KDSize size2 = KDFont::SmallFont->stringSize(line2);

  KDCoordinate totalHeight = size1.height() + 8 + size2.height();
  KDCoordinate startY = (bounds().height() - totalHeight) / 2;

  KDPoint p1((bounds().width() - size1.width()) / 2, startY);
  KDPoint p2((bounds().width() - size2.width()) / 2, startY + size1.height() + 8);

  ctx->drawString(line1, p1, KDFont::LargeFont, textColor, bgColor);
  ctx->drawString(line2, p2, KDFont::SmallFont, textColor, bgColor);
}

}
