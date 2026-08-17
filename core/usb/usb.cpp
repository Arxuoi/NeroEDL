#include "neroedl/usb.hpp"
#include <stdexcept>
#ifdef NEROEDL_HAS_LIBUSB
#include <libusb.h>
#endif
namespace neroedl {
#ifdef NEROEDL_HAS_LIBUSB
class LibusbBackend final : public UsbBackend {
  libusb_context *ctx_{};

public:
  LibusbBackend() {
    if (libusb_init(&ctx_) != 0)
      throw std::runtime_error("libusb initialization failed");
  }
  ~LibusbBackend() override { libusb_exit(ctx_); }
  std::vector<UsbDevice> enumerate() override {
    libusb_device **list{};
    const auto count = libusb_get_device_list(ctx_, &list);
    if (count < 0)
      throw std::runtime_error(libusb_error_name(static_cast<int>(count)));
    std::vector<UsbDevice> out;
    for (ssize_t n = 0; n < count; ++n) {
      libusb_device_descriptor dd{};
      if (libusb_get_device_descriptor(list[n], &dd))
        continue;
      UsbDevice d{};
      d.bus = libusb_get_bus_number(list[n]);
      d.address = libusb_get_device_address(list[n]);
      d.vid = dd.idVendor;
      d.pid = dd.idProduct;
      d.device_class = dd.bDeviceClass;
      d.device_subclass = dd.bDeviceSubClass;
      d.device_protocol = dd.bDeviceProtocol;
      libusb_device_handle *h{};
      if (libusb_open(list[n], &h) == 0) {
        auto str = [&](uint8_t idx) {
          unsigned char b[256]{};
          if (!idx)
            return std::string{};
          int z = libusb_get_string_descriptor_ascii(h, idx, b, sizeof b);
          return z > 0 ? std::string(reinterpret_cast<char *>(b),
                                     static_cast<size_t>(z))
                       : std::string{};
        };
        d.manufacturer = str(dd.iManufacturer);
        d.product = str(dd.iProduct);
        d.serial = str(dd.iSerialNumber);
        libusb_close(h);
      }
      for (uint8_t c = 0; c < dd.bNumConfigurations; ++c) {
        libusb_config_descriptor *cfg{};
        if (libusb_get_config_descriptor(list[n], c, &cfg))
          continue;
        for (uint8_t x = 0; x < cfg->bNumInterfaces; ++x)
          for (int a = 0; a < cfg->interface[x].num_altsetting; ++a) {
            const auto &id = cfg->interface[x].altsetting[a];
            Interface in{id.bInterfaceNumber,   id.bAlternateSetting,
                         id.bInterfaceClass,    id.bInterfaceSubClass,
                         id.bInterfaceProtocol, {}};
            for (uint8_t e = 0; e < id.bNumEndpoints; ++e)
              in.endpoints.push_back({id.endpoint[e].bEndpointAddress,
                                      id.endpoint[e].bmAttributes,
                                      id.endpoint[e].wMaxPacketSize});
            d.interfaces.push_back(std::move(in));
          }
        libusb_free_config_descriptor(cfg);
      }
      out.push_back(std::move(d));
    }
    libusb_free_device_list(list, 1);
    return out;
  }
};
#else
class UnavailableBackend final : public UsbBackend {
public:
  std::vector<UsbDevice> enumerate() override {
    throw std::runtime_error("libusb support was not compiled in; install "
                             "libusb-1.0 development files and rebuild");
  }
};
#endif
std::unique_ptr<UsbBackend> make_usb_backend() {
#ifdef NEROEDL_HAS_LIBUSB
  return std::make_unique<LibusbBackend>();
#else
  return std::make_unique<UnavailableBackend>();
#endif
}
} // namespace neroedl
