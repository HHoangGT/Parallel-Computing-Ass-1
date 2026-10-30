#include "mpi_common.h"

#include <chrono>
#include <thread>

void MpiBackoff::pause() {
    std::this_thread::sleep_for(std::chrono::microseconds(delay_us));
    if (delay_us < max_us) delay_us *= 2;
}
