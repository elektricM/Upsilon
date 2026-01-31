#include "rpi_controller.h"
#include "apps/apps_container.h"

namespace Rpi {

RpiController::RpiController() :
  ViewController(nullptr),
  m_rpiView()
{
}

View * RpiController::view() {
  return &m_rpiView;
}

bool RpiController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Back || event == Ion::Events::Home || event == Ion::Events::OK) {
    AppsContainer * container = AppsContainer::sharedAppsContainer();
    container->switchTo(container->appSnapshotAtIndex(0));
    return true;
  }
  return false;
}

}
