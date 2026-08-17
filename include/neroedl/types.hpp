#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace neroedl {
enum class DeviceState { adb, fastboot, edl, qualcomm_recovery, unknown };
enum class EntryStatus {
  supported,
  unsupported,
  failed,
  device_disconnected,
  already_edl,
  needs_physical_entry,
  unknown
};

struct Endpoint {
  std::uint8_t address{}, attributes{};
  std::uint16_t max_packet_size{};
};
struct Interface {
  std::uint8_t number{}, alternate{}, device_class{}, subclass{}, protocol{};
  std::vector<Endpoint> endpoints;
};
struct UsbDevice {
  std::uint8_t bus{}, address{};
  std::uint16_t vid{}, pid{};
  std::uint8_t device_class{}, device_subclass{}, device_protocol{};
  std::string manufacturer, product, serial;
  std::vector<Interface> interfaces;
};
struct ClassifiedDevice {
  UsbDevice usb;
  DeviceState state{DeviceState::unknown};
  std::string reason;
};
struct EntryResult {
  EntryStatus status{EntryStatus::unknown};
  std::string detail;
};

std::string to_string(DeviceState state);
std::string to_string(EntryStatus status);
} // namespace neroedl
