#include "suspend_timer.h"
#include "apps_container.h"
#ifdef ENABLE_RPI
#include <ion/rpi.h>
#endif

SuspendTimer::SuspendTimer() :
  Timer(GlobalPreferences::sharedGlobalPreferences()->idleBeforeSuspendSeconds()*1000/Timer::TickDuration)
{
}

bool SuspendTimer::fire() {
#ifdef ENABLE_RPI
  if (Ion::Rpi::isPowered()) {
    return false;
  }
#endif
  /* We could just call container->suspend(), but we want to notify all
   * responders in the responder chain that the calculator will be switched off,
   * so we use an event to switch off the calculator. */
  AppsContainer * container = AppsContainer::sharedAppsContainer();
  container->dispatchEvent(Ion::Events::OnOff);
  return false;
}
