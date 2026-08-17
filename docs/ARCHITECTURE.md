# Architecture

`UsbBackend` adalah seam utama: backend libusb menghasilkan value object descriptor
tanpa mempertahankan handle mentah. `UsbMonitor` membandingkan snapshot sehingga
disconnect tidak menyebabkan penggunaan pointer libusb yang sudah invalid.
`classify()` murni dan konservatif. Entry engine menerima hasil klasifikasi,
memanggil `probe()` sebelum `enter()`, lalu memverifikasi re-enumeration 9008.

Sahara dipisahkan menjadi parser bounded dan `Transport`. Parser memvalidasi header,
ukuran minimum, batas 1 MiB, exact framing, command, dan ukuran HELLO sebelum field
dibaca. Handshake hanya mengirim HELLO_RESP; tidak memilih/mengirim programmer.
Firehose kelak harus menjadi modul terpisah di belakang policy yang melarang write
default, dan tidak boleh mengakali signature/authentication OEM.

## Invariants

1. Probe tidak mengirim USB transfer selain standard descriptor reads.
2. Unknown tetap unknown; VID/vendor string bukan bukti capability entry.
3. Exit sukses entry memerlukan state already-EDL atau observasi `05c6:9008` baru.
4. Semua protocol transfer kelak wajib memakai timeout dan mengembalikan disconnect.
5. Pemilihan multi-device ditolak, bukan memilih target secara arbitrer.

## Roadmap

Hubungkan transport bulk libusb Sahara dengan interface/endpoint discovery dan claim
yang eksplisit; hotplug callback native; selector bus/serial; Windows CI; structured
JSON diagnostics; corpus packet fuzzing; lalu Firehose read-only (`getstorageinfo`)
hanya setelah spesifikasi dan perangkat uji tersedia.
