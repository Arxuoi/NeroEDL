#pragma once
#include <string>
#include <vector>
namespace neroedl {
struct DeviceRecord {
  std::string vendor, model, soc, status;
  std::vector<std::string> usb_ids, entry_methods;
};
class DeviceDatabase {
public:
  static DeviceDatabase load(const std::string &path);
  const std::vector<DeviceRecord> &records() const { return records_; }

private:
  std::vector<DeviceRecord> records_;
};
} // namespace neroedl
