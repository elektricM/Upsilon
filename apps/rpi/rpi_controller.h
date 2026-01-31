#ifndef APPS_RPI_CONTROLLER_H
#define APPS_RPI_CONTROLLER_H

#include <escher.h>
#include "rpi_view.h"

namespace Rpi {

class RpiController : public ViewController {
public:
  RpiController();
  View * view() override;
  bool handleEvent(Ion::Events::Event event) override;
private:
  RpiView m_rpiView;
};

}

#endif
