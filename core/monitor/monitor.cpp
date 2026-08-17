#include "neroedl/usb.hpp"
#include <map>
#include <thread>
namespace neroedl {
namespace {
auto key(const UsbDevice &d) { return std::pair{d.bus, d.address}; }
} // namespace
void UsbMonitor::watch(
    const std::function<bool(const char *, const UsbDevice &)> &cb,
    std::chrono::milliseconds interval) {
  std::map<std::pair<uint8_t, uint8_t>, UsbDevice> old;
  for (const auto &d : backend_.enumerate())
    old[key(d)] = d;
  for (;;) {
    std::this_thread::sleep_for(interval);
    std::map<std::pair<uint8_t, uint8_t>, UsbDevice> now;
    for (const auto &d : backend_.enumerate())
      now[key(d)] = d;
    for (const auto &[k, d] : old)
      if (!now.contains(k) && !cb("detached", d))
        return;
    for (const auto &[k, d] : now)
      if (!old.contains(k) && !cb("attached", d))
        return;
    old = std::move(now);
  }
}
std::optional<UsbDevice>
UsbMonitor::wait_for(uint16_t vid, uint16_t pid,
                     std::chrono::milliseconds timeout,
                     std::chrono::milliseconds interval) {
  auto end = std::chrono::steady_clock::now() + timeout;
  while (std::chrono::steady_clock::now() < end) {
    for (auto &d : backend_.enumerate())
      if (d.vid == vid && d.pid == pid)
        return d;
    std::this_thread::sleep_for(interval);
  }
  return std::nullopt;
}
} // namespace neroedl
