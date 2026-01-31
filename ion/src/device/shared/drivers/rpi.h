#ifndef ION_DEVICE_SHARED_RPI_H
#define ION_DEVICE_SHARED_RPI_H

#include <regs/regs.h>

namespace Ion {
namespace Rpi {
namespace Device {

using namespace Ion::Device::Regs;

constexpr static GPIOPin PowerPin(GPIOB, 9);
constexpr static GPIO ChipSelectGPIO = GPIOA;
constexpr static uint8_t ChipSelectPin = 6;
constexpr static DMA DMAEngine = DMA2;
constexpr static int DMAStream = 2;

void init();
void shutdown();

}
}
}

extern "C" void rpi_isr();

#endif
