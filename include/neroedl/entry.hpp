#pragma once
#include "neroedl/types.hpp"
#include "neroedl/usb.hpp"
#include <memory>
namespace neroedl {
class EdlEntryMethod {
public:
  virtual ~EdlEntryMethod() = default;
  virtual bool probe(const ClassifiedDevice &) = 0;
  virtual EntryResult enter(const ClassifiedDevice &) = 0;
  virtual const char *name() const = 0;
};
std::vector<std::unique_ptr<EdlEntryMethod>> entry_methods();
EntryResult enter_auto(const ClassifiedDevice &, UsbBackend &, bool verbose);
} // namespace neroedl
