#include "neroedl/classifier.hpp"
#include "neroedl/database.hpp"
#include "neroedl/entry.hpp"
#include "neroedl/log.hpp"
#include "neroedl/usb.hpp"
#include <atomic>
#include <csignal>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
using namespace neroedl;
namespace {
std::atomic_bool running = true;
void stop(int) { running = false; }
std::string id(uint16_t v, uint16_t p) {
  std::ostringstream s;
  s << std::hex << std::setfill('0') << std::setw(4) << v << ':' << std::setw(4)
    << p;
  return s.str();
}
void print(const UsbDevice &d) {
  auto c = classify(d);
  std::cout << "[+] Bus/address   : " << unsigned(d.bus) << '/'
            << unsigned(d.address)
            << "\n[+] VID:PID       : " << id(d.vid, d.pid)
            << "\n[+] Manufacturer  : "
            << (d.manufacturer.empty() ? "<unavailable>" : d.manufacturer)
            << "\n[+] Product       : "
            << (d.product.empty() ? "<unavailable>" : d.product)
            << "\n[+] Current state : " << to_string(c.state)
            << "\n[*] Evidence      : " << c.reason << '\n';
  for (const auto &i : d.interfaces) {
    std::cout << "    interface " << unsigned(i.number) << " alt "
              << unsigned(i.alternate) << " class/subclass/protocol "
              << std::hex << std::setfill('0') << std::setw(2)
              << unsigned(i.device_class) << '/' << std::setw(2)
              << unsigned(i.subclass) << '/' << std::setw(2)
              << unsigned(i.protocol) << std::dec << '\n';
    for (const auto &e : i.endpoints)
      std::cout << "      endpoint 0x" << std::hex << unsigned(e.address)
                << " attributes 0x" << unsigned(e.attributes) << " max-packet "
                << std::dec << e.max_packet_size << '\n';
  }
}
void usage() {
  std::cout << "Usage: neroedl [--verbose] "
               "<probe|info|watch|enter|devices|diagnose> [--auto]\n";
}
} // namespace
int main(int argc, char **argv) {
  bool verbose = false;
  std::string command;
  for (int i = 1; i < argc; ++i)
    if (std::string(argv[i]) == "--verbose")
      verbose = true;
    else if (std::string(argv[i]) != "--auto")
      command = argv[i];
  log::set_verbose(verbose);
  std::cout << "NeroEDL 0.1.0\nButtonless Qualcomm Recovery Toolkit\n\n";
  if (command.empty()) {
    usage();
    return 2;
  }
  try {
    if (command == "info") {
      std::cout << "Platform: Linux x86_64 primary\nBackend: libusb "
                   "1.0\nSafety: no flashing, storage writes, authentication "
                   "bypass, or brute-force requests\n";
      return 0;
    }
    if (command == "devices") {
      std::string path = "devices/database.json";
      if (!std::filesystem::exists(path))
        path = "/usr/local/share/neroedl/database.json";
      auto db = DeviceDatabase::load(path);
      for (const auto &r : db.records())
        std::cout << r.vendor << ' ' << r.model << " | SoC " << r.soc << " | "
                  << r.status << '\n';
      return 0;
    }
    auto usb = make_usb_backend();
    if (command == "probe") {
      auto ds = usb->enumerate();
      if (ds.empty())
        log::error("No USB devices enumerated");
      for (const auto &d : ds) {
        print(d);
        std::cout << '\n';
      }
      return 0;
    }
    if (command == "watch") {
      std::signal(SIGINT, stop);
      log::info("Watching USB changes; press Ctrl-C to stop");
      UsbMonitor m(*usb);
      m.watch([](const char *event, const UsbDevice &d) {
        std::cout << "[+] " << event << " " << id(d.vid, d.pid) << '\n';
        return running.load();
      });
      return 0;
    }
    if (command == "diagnose") {
      auto ds = usb->enumerate();
      if (ds.empty()) {
        log::error("No usable USB interface detected");
        std::cout << "\nPossible causes:\n - device is not receiving power\n - "
                     "USB data lines unavailable\n - PBL did not expose a "
                     "host-accessible interface\n - physical EDL trigger may "
                     "be required\n\nSoftware-only EDL entry cannot be "
                     "performed in the current state.\n";
        return 1;
      }
      log::success("Electrical USB connection and enumeration detected");
      bool useful = false;
      for (const auto &d : ds) {
        auto c = classify(d);
        if (c.state != DeviceState::unknown) {
          useful = true;
          print(d);
        }
      }
      if (!useful) {
        log::error("No recognized recovery communication path; unknown devices "
                   "are not considered EDL-capable");
        return 1;
      }
      return 0;
    }
    if (command == "enter") {
      auto ds = usb->enumerate();
      std::vector<ClassifiedDevice> candidates;
      for (auto &d : ds) {
        auto c = classify(d);
        if (c.state != DeviceState::unknown)
          candidates.push_back(std::move(c));
      }
      if (candidates.size() != 1) {
        log::error(candidates.empty()
                       ? "No recognized candidate device"
                       : "Multiple candidate devices; isolate one device "
                         "before state-changing operations");
        return 1;
      }
      log::info("Capability probing selected entry method...");
      auto r = enter_auto(candidates.front(), *usb, verbose);
      std::cout << "[+] Entry status  : " << to_string(r.status)
                << "\n[*] Detail        : " << r.detail << '\n';
      return r.status == EntryStatus::supported ||
                     r.status == EntryStatus::already_edl
                 ? 0
                 : 1;
    }
    usage();
    return 2;
  } catch (const std::exception &e) {
    log::error(e.what());
    return 1;
  }
}
