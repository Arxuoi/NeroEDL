#pragma once
#include "neroedl/types.hpp"
#include <chrono>
#include <functional>
#include <memory>

namespace neroedl {
class UsbBackend {
public:
  virtual ~UsbBackend() = default;
  virtual std::vector<UsbDevice> enumerate() = 0;
};
std::unique_ptr<UsbBackend> make_usb_backend();

class UsbMonitor {
public:
  explicit UsbMonitor(UsbBackend &backend) : backend_(backend) {}
  void
  watch(const std::function<bool(const char *, const UsbDevice &)> &callback,
        std::chrono::milliseconds interval = std::chrono::milliseconds(500));
  std::optional<UsbDevice>
  wait_for(std::uint16_t vid, std::uint16_t pid,
           std::chrono::milliseconds timeout,
           std::chrono::milliseconds interval = std::chrono::milliseconds(100));

private:
  UsbBackend &backend_;
};
} // namespace neroedl
