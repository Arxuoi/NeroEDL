# NeroEDL

**NeroEDL 0.1.0 — Buttonless Qualcomm Recovery Toolkit** adalah CLI C++20 untuk
mengobservasi jalur recovery USB yang *masih diekspos* oleh perangkat Qualcomm.
Fokus rilis awal ini adalah enumerasi yang aman, klasifikasi konservatif,
monitor re-enumeration, diagnosis hardbrick, entry berbasis capability, dan parser
handshake Sahara yang dapat diuji tanpa telepon.

> **NeroEDL tidak dapat secara ajaib memasukkan setiap hardbricked Qualcomm ke
> EDL.** Jika SoC tidak mengekspos interface USB yang dapat menerima komunikasi,
> atau hardware mensyaratkan tombol/test point agar PBL memilih EDL, software host
> tidak dapat menggantikan koneksi elektrik tersebut. NeroEDL hanya menghilangkan
> kebutuhan itu ketika masih ada recovery path yang dapat diakses software.

## Yang benar-benar tersedia

* Enumerasi descriptor device/configuration/interface/endpoint dengan libusb,
  termasuk string descriptor yang mungkin gagal karena permission.
* Klasifikasi konservatif ADB, Fastboot, `05c6:9008`, Qualcomm non-9008, dan
  unknown. VID Qualcomm non-9008 **tidak** otomatis dianggap EDL-capable.
* Watcher polling snapshot berdasarkan bus/address, serta waiter re-enumeration.
* `enter`: existing-EDL; ADB hanya setelah `adb get-state`; Fastboot diprobe tetapi
  ditolak karena tidak ada command EDL standar/capability advertisement. Setelah
  ADB menerima perintah, sukses hanya dilaporkan jika `05c6:9008` benar-benar
  muncul. Tidak ada vendor control request yang ditebak atau di-brute-force.
* Parser Sahara bounded dan handshake HELLO/HELLO_RESP dasar melalui abstraction
  transport. Transfer programmer dan Firehose sengaja belum dilakukan.
* Database dengan status eksplisit. Entri generik saat ini **UNTESTED**; tidak ada
  model atau SoC yang diklaim TESTED.

Tidak ada flashing, write storage, penghapusan userdata, bypass authentication,
exploit Secure Boot, programmer proprietary, atau perintah destruktif.

## Build dan penggunaan

```sh
sudo apt install cmake ninja-build g++ pkg-config libusb-1.0-0-dev
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure

./build/neroedl probe
./build/neroedl info
./build/neroedl watch
./build/neroedl enter --auto
./build/neroedl devices
./build/neroedl diagnose
./build/neroedl --verbose probe
```

Jika header libusb tidak ditemukan, build tetap menghasilkan binary yang gagal
dengan pesan eksplisit saat USB dipakai; ini menjaga unit test parser tetap
reproducible. Build hardware-capable **wajib** menemukan libusb. Akses descriptor
string juga mungkin memerlukan aturan udev; jangan menjalankan tool sebagai root
tanpa memahami risikonya.

Sanitizer: `cmake -S . -B build-san -G Ninja -DNEROEDL_SANITIZERS=ON`.

## State transition

```text
USB CONNECT
    │
    ▼
ENUMERATE
    │
    ├── ADB ────────────┐
    ├── FASTBOOT ───────┤
    ├── OEM/QUALCOMM ───┤ → ENTRY ENGINE
    │                   │
    ├── 9008 ───────────┴→ EDL READY
    │
    └── NO INTERFACE ─────→ PHYSICAL ENTRY MAY BE REQUIRED
```

## Dukungan dan batas kompatibilitas

| Lapisan | Status 0.1.0 |
|---|---|
| Linux x86_64 + libusb | Implemented, belum divalidasi pada hardware oleh project |
| Windows x64 | Target desain; backend/build belum divalidasi |
| ADB entry | Experimental; hanya device yang menerima `reboot edl` |
| Fastboot entry | Capability probe saja; transisi ditolak dengan aman |
| Qualcomm 9008 detection | Descriptor/VID:PID implemented |
| Sahara | Parser + HELLO response + mock tests; transport USB belum dihubungkan |
| Firehose | Batas modul didokumentasikan; belum diimplementasikan |
| Perangkat/SoC tertentu | Tidak ada yang diklaim supported/tested |

Lihat [arsitektur](docs/ARCHITECTURE.md), [riset](docs/RESEARCH.md), dan
[kebijakan keamanan](docs/SAFETY.md). Kontribusi trace descriptor anonim dan hasil
pengujian hardware sangat diharapkan; jangan kirim programmer OEM berhak cipta.

## Lisensi

Kode project: Apache-2.0. Dependency runtime libusb adalah LGPL-2.1-or-later.
Tool eksternal opsional `adb`/`fastboot` berasal dari Android platform-tools dan
tidak didistribusikan oleh project ini.
