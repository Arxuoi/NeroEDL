# Safety and threat model

NeroEDL menganggap perangkat dan packet masukannya tidak tepercaya. Panjang packet
divalidasi sebelum alokasi/akses. Timeout wajib ada pada API transport. Rilis ini
tidak menulis storage atau mengirim programmer.

`enter` memang mengubah state. Gunakan hanya pada hardware milik sendiri, lepaskan
device USB lain, dan pastikan daya stabil. ADB command mungkin didukung, diabaikan,
atau membuat device reboot ke mode selain EDL; karena itu command accepted bukan
sukses—re-enumeration 9008 adalah bukti yang diperlukan.

Project tidak menerima fitur untuk bypass Secure Boot, authentication, signature,
anti-rollback, atau lock OEM. Jalur tanpa interface host-accessible harus dilaporkan
`NEEDS_PHYSICAL_ENTRY`, bukan dicoba dengan request undocumented.
