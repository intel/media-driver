---
name: Bug Issue
about: Use this template for reporting a bug
---

## System information
- CPU information(`cat /proc/cpuinfo | grep "model name" | uniq`):
- GPU information(`lspci -nn | grep -E 'VGA|isplay`):
- Display server if rendering to display(X or wayland):

## Issue behavior
### Describe the current behavior

### Describe the expected behavior

## Debug information
- What's libva/libva-utils/gmmlib/media-driver version?
- Could you confirm whether GPU hardware exist or not by `ls /dev/dri`?
- Could you provide vainfo log if possible by `vainfo >vainfo.log 2>&1`?
- Could you provide libva trace log if possible? Run cmd `export LIBVA_TRACE=/tmp/libva_trace.log` first then execute the case.
- Could you attach dmesg log if it's GPU hang by `dmesg >dmesg.log 2>&1`?
- Do you want to contribute a patch to fix the issue? (yes/no):
