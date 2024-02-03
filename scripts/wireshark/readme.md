# Wireshark Dissector for 3DP Axxel

The Lua dissector `3dpaxxel.lua` decodes messages transmitted to and received from the accelerometer controller. 
The dissector may be used with `wireshark` or `tshark`.

## Usage:

```bash
sudo modprobe usbmon

# optional: allow non-superuser to capture packages (requires new login: i.e. restart X11 etc...)
sudo dpkg-reconfigure wireshark-common
sudo usermod -a -G wireshark $USER

./wireshark.sh tshark    # starts a tshark capture (alternatively with `sudo`)
./wireshark.sh wireshark # starts a wireshark capture (alternatively with `sudo`)
./wireshark.sh --help    # for more details
```
