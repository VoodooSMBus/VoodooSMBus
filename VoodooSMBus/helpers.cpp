#include "helpers.hpp"

uint64_t clock_get_uptime_nanoseconds() {
    AbsoluteTime timestamp;
    clock_get_uptime(&timestamp);
    uint64_t timestamp_ns;
    absolutetime_to_nanoseconds(timestamp, &timestamp_ns);
    return timestamp_ns;
}
