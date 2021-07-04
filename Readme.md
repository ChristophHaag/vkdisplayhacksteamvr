# vkdisplayinfo

Print displays and modes enumerated with the Vulkan function

`vkGetPhysicalDeviceDisplayPropertiesKHR`

## radv note:

Note that radv will not enumerate any displays if some other application holds drm master.
This means, it will not print anything when run under X11, but will when run on a tty.

The proprietary nvidia driver has no such reservations, it will always enumerate all connected displays.

# Build and run:

```
meson build
ninja -C build
build/vkdisplayinfo
```
