#include "neroedl/classifier.hpp"
#include "neroedl/database.hpp"
#include "neroedl/sahara.hpp"
#include "neroedl/usb.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace neroedl;
namespace {
int failures = 0;
void check(bool v, const char *n) {
  if (!v) {
    std::cerr << "FAIL: " << n << '\n';
    ++failures;
  }
}
void put(std::vector<uint8_t> &b, uint32_t v) {
  for (int i = 0; i < 4; ++i)
    b.push_back(v >> (i * 8));
}
class MockUsb : public UsbBackend {
public:
  std::vector<std::vector<UsbDevice>> frames;
  size_t n{};
  std::vector<UsbDevice> enumerate() override {
    if (frames.empty())
      throw std::runtime_error("disconnect");
    return frames[std::min(n++, frames.size() - 1)];
  }
};
class MockSahara : public sahara::Transport {
public:
  std::vector<uint8_t> input, written;
  bool timeout{};
  std::vector<uint8_t> read(std::chrono::milliseconds) override {
    if (timeout)
      throw std::runtime_error("timeout");
    return input;
  }
  void write(std::span<const uint8_t> b, std::chrono::milliseconds) override {
    written.assign(b.begin(), b.end());
  }
};
} // namespace
int main(int argc, char **argv) {
  UsbDevice edl{};
  edl.vid = 0x05c6;
  edl.pid = 0x9008;
  check(classify(edl).state == DeviceState::edl,
        "VID/PID database and EDL classifier");
  UsbDevice unknown{};
  unknown.vid = 0x1234;
  check(classify(unknown).state == DeviceState::unknown,
        "unknown conservative classifier");
  UsbDevice adb{};
  adb.interfaces.push_back({0, 0, 0xff, 0x42, 1, {}});
  check(classify(adb).state == DeviceState::adb, "descriptor classifier");
  std::vector<uint8_t> hello;
  put(hello, 1);
  put(hello, 0x30);
  put(hello, 2);
  put(hello, 1);
  put(hello, 4096);
  put(hello, 0);
  while (hello.size() < 0x30)
    put(hello, 0);
  auto p = sahara::parse(hello);
  check(sahara::parse_hello(p).max_packet == 4096, "Sahara HELLO parser");
  MockSahara s;
  s.input = hello;
  check(sahara::handshake(s, std::chrono::milliseconds(10)).detected &&
            s.written.size() == 0x30,
        "Sahara handshake");
  try {
    sahara::parse(std::vector<uint8_t>{1, 2});
    check(false, "malformed packet");
  } catch (const std::invalid_argument &) {
  }
  MockSahara t;
  t.timeout = true;
  check(!sahara::handshake(t, std::chrono::milliseconds(1)).detected,
        "transport timeout");
  MockUsb usb;
  usb.frames = {{{}}, {{edl}}};
  UsbMonitor m(usb);
  check(m.wait_for(0x05c6, 0x9008, std::chrono::milliseconds(30),
                   std::chrono::milliseconds(1))
            .has_value(),
        "re-enumeration");
  MockUsb disconnected;
  try {
    disconnected.enumerate();
    check(false, "disconnect");
  } catch (const std::runtime_error &) {
  }
  if (argc > 1)
    check(DeviceDatabase::load(argv[1]).records().size() == 1,
          "device database parser");
  else
    check(false, "database path");
  if (failures)
    return 1;
  std::cout << "all tests passed\n";
}
