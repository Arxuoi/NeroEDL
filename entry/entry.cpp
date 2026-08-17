#include "neroedl/entry.hpp"
#include "neroedl/log.hpp"
#include <array>
#include <cstdio>
#include <sys/wait.h>
namespace neroedl {
namespace {
struct Result {
  int code;
  std::string output;
};
Result run(const char *cmd) {
  std::array<char, 256> b{};
  std::string out;
  FILE *p = popen(cmd, "r");
  if (!p)
    return {-1, {}};
  while (fgets(b.data(), static_cast<int>(b.size()), p))
    out += b.data();
  int s = pclose(p);
  return {WIFEXITED(s) ? WEXITSTATUS(s) : -1, out};
}
class Adb final : public EdlEntryMethod {
public:
  bool probe(const ClassifiedDevice &d) override {
    if (d.state != DeviceState::adb)
      return false;
    auto r = run("adb get-state 2>/dev/null");
    return r.code == 0 && r.output.find("device") != std::string::npos;
  }
  EntryResult enter(const ClassifiedDevice &) override {
    auto r = run("adb reboot edl 2>/dev/null");
    return r.code == 0
               ? EntryResult{EntryStatus::device_disconnected,
                             "ADB accepted reboot edl; USB outcome must be "
                             "verified"}
               : EntryResult{EntryStatus::failed,
                             "ADB rejected or could not deliver reboot edl"};
  }
  const char *name() const override { return "ADB reboot-edl"; }
};
class Fastboot final : public EdlEntryMethod {
public:
  bool probe(const ClassifiedDevice &d) override {
    return d.state == DeviceState::fastboot &&
           run("fastboot getvar product 2>&1").code == 0;
  }
  EntryResult enter(const ClassifiedDevice &) override {
    return {EntryStatus::unsupported,
            "No standardized fastboot capability advertises EDL; refusing "
            "vendor-specific commands"};
  }
  const char *name() const override { return "Fastboot capability probe"; }
};
class Already final : public EdlEntryMethod {
public:
  bool probe(const ClassifiedDevice &d) override {
    return d.state == DeviceState::edl;
  }
  EntryResult enter(const ClassifiedDevice &) override {
    return {EntryStatus::already_edl, "Device already enumerates as 05c6:9008"};
  }
  const char *name() const override { return "Existing EDL"; }
};
} // namespace
std::vector<std::unique_ptr<EdlEntryMethod>> entry_methods() {
  std::vector<std::unique_ptr<EdlEntryMethod>> v;
  v.push_back(std::make_unique<Already>());
  v.push_back(std::make_unique<Adb>());
  v.push_back(std::make_unique<Fastboot>());
  return v;
}
EntryResult enter_auto(const ClassifiedDevice &d, UsbBackend &usb,
                       bool verbose) {
  for (auto &m : entry_methods()) {
    if (verbose)
      log::debug(std::string("probing method: ") + m->name());
    if (!m->probe(d))
      continue;
    auto r = m->enter(d);
    if (r.status == EntryStatus::device_disconnected) {
      log::info("Waiting for USB re-enumeration (10 seconds)...");
      UsbMonitor monitor(usb);
      if (monitor.wait_for(0x05c6, 0x9008, std::chrono::seconds(10)))
        return {EntryStatus::supported, "EDL transition verified as 05c6:9008"};
      return {
          EntryStatus::failed,
          "command was accepted but 05c6:9008 did not appear before timeout"};
    }
    return r;
  }
  return {d.state == DeviceState::unknown ? EntryStatus::needs_physical_entry
                                          : EntryStatus::unsupported,
          "No capability-validated software entry method is available"};
}
} // namespace neroedl
