#include "neroedl/classifier.hpp"
namespace neroedl {
std::string to_string(DeviceState s) {
  switch (s) {
  case DeviceState::adb:
    return "ADB";
  case DeviceState::fastboot:
    return "Fastboot";
  case DeviceState::edl:
    return "Qualcomm EDL (9008)";
  case DeviceState::qualcomm_recovery:
    return "Qualcomm diagnostic/recovery interface";
  default:
    return "Unknown USB state";
  }
}
std::string to_string(EntryStatus s) {
  switch (s) {
  case EntryStatus::supported:
    return "SUPPORTED";
  case EntryStatus::unsupported:
    return "UNSUPPORTED";
  case EntryStatus::failed:
    return "FAILED";
  case EntryStatus::device_disconnected:
    return "DEVICE_DISCONNECTED";
  case EntryStatus::already_edl:
    return "ALREADY_EDL";
  case EntryStatus::needs_physical_entry:
    return "NEEDS_PHYSICAL_ENTRY";
  default:
    return "UNKNOWN";
  }
}
ClassifiedDevice classify(const UsbDevice &d) {
  if (d.vid == 0x05c6 && d.pid == 0x9008)
    return {d, DeviceState::edl, "canonical Qualcomm QDLoader 9008 identifier"};
  for (const auto &i : d.interfaces) {
    if (i.device_class == 0xff && i.subclass == 0x42 && i.protocol == 0x01)
      return {d, DeviceState::adb, "Android USB ADB interface descriptor"};
    if (i.device_class == 0xff && i.subclass == 0x42 && i.protocol == 0x03)
      return {d, DeviceState::fastboot,
              "Android USB fastboot interface descriptor"};
  }
  if (d.vid == 0x18d1 && (d.product.find("Fastboot") != std::string::npos ||
                          d.product.find("fastboot") != std::string::npos))
    return {d, DeviceState::fastboot, "Google VID and product string"};
  if (d.vid == 0x05c6)
    return {d, DeviceState::qualcomm_recovery,
            "Qualcomm VID; capability is not assumed"};
  return {d, DeviceState::unknown, "no conservative classifier rule matched"};
}
} // namespace neroedl
