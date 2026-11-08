# ZTop ZT9101 Wireless LAN Driver

Linux kernel driver for ZTop ZT9101 chipset based USB WiFi adapters.

**Fork with Linux kernel 7.1+ support, stability fixes, and DKMS integration.**

## Changes in this fork

- **Kernel 7.1 API compatibility** — adapted 8 cfg80211 callbacks to the new
  `struct wireless_dev *wdev` API with backward compatibility via `#if` guards
- **Stability fixes** — removed NULL dereference (`os/linux/rx_linux.c`),
  double-free (`os/linux/hif/hif_queue.c`), memory leak (`os/linux/hif/hif.c`),
  and added proper error path cleanup in `hif_dev_insert()`
- **Activated suspend/resume callbacks** in `os/linux/hif/usb.c`
- **WEXT stubs** for distributions that lack `iwe_stream_add_point`/`iwe_stream_add_event` (Fedora)
- **DKMS support** — automatic module rebuild on kernel updates

## Requirements

- Linux kernel headers for the target kernel (`kernel-devel` on Fedora/RHEL,
  `linux-headers` on Debian/Ubuntu)
- DKMS (optional, for automatic rebuilds)
- GCC and make

## Building and Installing

### Option 1: DKMS (recommended)

DKMS automatically rebuilds the module when the kernel is updated.

```bash
cd ZT9101_android_V1.2.1800.20241115/
sudo dkms add .
sudo dkms build zt9101_ztopmac_usb/1.2.1800
sudo dkms install zt9101_ztopmac_usb/1.2.1800 -k $(uname -r)
```

### Option 2: Manual build

```bash
cd ZT9101_android_V1.2.1800.20241115/
make
sudo make install
sudo depmod -a
```

### Load the module

```bash
sudo modprobe zt9101_ztopmac_usb
```

### Optional: Auto-load at boot

```bash
echo "zt9101_ztopmac_usb" | sudo tee /etc/modules-load.d/zt9101_ztopmac_usb.conf
```

### Module parameters

A configuration file can be passed via the `cfg` parameter:

```bash
options zt9101_ztopmac_usb cfg=/etc/zt9101_wifi.cfg
```

Place this in `/etc/modprobe.d/zt9101_ztopmac_usb.conf`.

The default configuration file in the driver source (`wifi.cfg`) has sane defaults
for station mode with WPA2.

## Compatibility

| Kernel Version | Status |
|---|---|
| 7.x | Tested |
| 6.x | Tested |
| 5.15.x | Tested |
| 5.10.x | Tested |
| 5.5.x| Not worked (struct proc_ops changed) |
## Driver Structure

The driver source is in the `ZT9101_android_V1.2.1800.20241115/` subdirectory:

```
ZT9101_android_V1.2.1800.20241115/
├── Makefile          # Top-level build file
├── dkms.conf         # DKMS configuration
├── mak/              # Build system helpers
├── platform/         # Platform-specific settings
├── os/               # OS-specific code (Linux USB, cfg80211)
├── nic/              # Network interface controller logic
├── fw/               # Firmware binaries
├── doc/              # Documentation
├── script/           # Build scripts
└── wifi.cfg          # Default configuration
```

## License

GPL — see `LICENSE` file for details.

Original copyright (C) 2021 Shandong ZTop Microelectronics Co., Ltd.
