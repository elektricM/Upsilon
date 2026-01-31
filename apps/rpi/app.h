#ifndef RPI_APP_H
#define RPI_APP_H

#include <escher.h>
#include "rpi_controller.h"

namespace Rpi {

class App : public ::App {
public:
  class Descriptor : public ::App::Descriptor {
  public:
    I18n::Message name() override;
    I18n::Message upperName() override;
    const Image * icon() override;
  };
  class Snapshot : public ::App::Snapshot {
  public:
    App * unpack(Container * container) override;
    Descriptor * descriptor() override;
  };
  void didBecomeActive(Window * window) override;
private:
  App(Snapshot * snapshot);
  RpiController m_rpiController;
};

}

#endif
