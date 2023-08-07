# vkdisplayhacksteamvr

# Build:

```
meson build
ninja -C build
```

# Usage:

Step 1: Run
```
build/vkdisplayhacksteamvr
```

Take note of the available xrandr output names like `DP-1`.

Step 2: Set environment variables and run SteamVR:
```
export VK_DISPLAY_HACK_STEAMVR=DP-1 # adjust for your desired output
export VK_LAYER_PATH="$PWD/build" # absolute path to your build dir
export VK_INSTANCE_LAYERS=VK_LAYER_HAAGCH_vkdisplayhacksteamvr
# export VK_LOADER_DEBUG=all # Optional: Debug output to see if the layer is working or why not.

~/.steam/steam/steamapps/common/SteamVR/bin/vrstartup.sh
```

