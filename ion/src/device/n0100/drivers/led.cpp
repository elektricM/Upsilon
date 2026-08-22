#include <ion/led.h>
#include <ion/usb.h>
#include <ion/exam_mode.h>

namespace Ion {
namespace LED {

KDColor updateColorWithPlugAndCharge() {
  KDColor ledColor = getColor();
  if (ExamMode::FetchExamMode() == 0) {
    ledColor = USB::isPlugged() ? KDColor::RGB24(0xCCDDFF) : KDColorBlack;
    setColor(ledColor);
  }
  return ledColor;
}

}
}
