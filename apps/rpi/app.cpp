#include "app.h"
#include "rpi_icon.h"
#include "apps/apps_container.h"
#include "apps/i18n.h"
#include <ion/rpi.h>

namespace Rpi {

I18n::Message App::Descriptor::name() {
  return I18n::Message::RpiApp;
}

I18n::Message App::Descriptor::upperName() {
  return I18n::Message::RpiAppCapital;
}

const Image * App::Descriptor::icon() {
  return ImageStore::RpiIcon;
}

App * App::Snapshot::unpack(Container * container) {
  return new (container->currentAppBuffer()) App(this);
}

App::Descriptor * App::Snapshot::descriptor() {
  static Descriptor descriptor;
  return &descriptor;
}

App::App(Snapshot * snapshot) :
  ::App(snapshot, &m_rpiController),
  m_rpiController()
{
}

void App::didBecomeActive(Window * window) {
  ::App::didBecomeActive(window);
  Ion::Rpi::transferControl();
  AppsContainer * container = AppsContainer::sharedAppsContainer();
  container->switchTo(container->appSnapshotAtIndex(0));
}

}
