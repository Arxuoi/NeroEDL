# Research notes and provenance

Implementasi ditulis dari model packet dan API publik, bukan menyalin source pihak
ketiga. Sumber yang ditinjau pada 2026-08-17:

* [libusb descriptor API](https://libusb.sourceforge.io/api-1.0/group__libusb__desc.html)
  dan [hotplug API](https://libusb.sourceforge.io/api-1.0/group__libusb__hotplug.html):
  ownership descriptor/list, enumeration, string access, dan lifecycle device.
* [Android fastboot documentation](https://source.android.com/docs/core/architecture/bootloader/fastbootd):
  membedakan fastboot bootloader/userspace; tidak menetapkan command EDL portable.
* [linux-msm/qdl](https://github.com/linux-msm/qdl) (BSD-3-Clause): referensi silang
  legal untuk framing Sahara/Firehose pada ekosistem Linux Qualcomm.
* [bkerler/edl](https://github.com/bkerler/edl) (GPL-3.0): perilaku interoperabilitas
  publik dan gambaran programmer/Sahara/Firehose. Tidak ada source yang disalin.
* Linux kernel documentation untuk
  [USB userspace APIs](https://docs.kernel.org/driver-api/usb/usb.html) dan Android
  Open Source Project untuk konsep bootloader.

Kesimpulan: PBL menentukan apakah EDL diekspos; host tidak dapat menciptakan jalur
elektrik yang tidak ada. Sahara adalah tahap loader awal, sedangkan Firehose berjalan
setelah programmer yang sesuai dan dipercaya device. Programmer bukan universal dan
Secure Boot tetap harus dihormati. Karena bukti command vendor sering anecdotal dan
tidak mengiklankan capability, versi ini sengaja tidak mengimplementasikan request
Qualcomm/OEM undocumented.

Riset lanjutan memerlukan capture USB dari perangkat milik contributor, variasi
HELLO legal, dokumentasi OEM yang boleh didistribusikan, dan matriks hasil nyata yang
memisahkan kemampuan SoC, implementasi OEM, serta perilaku model/firmware.
