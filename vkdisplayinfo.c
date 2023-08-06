// Copyright 2019-2022, Collabora, Ltd.
// SPDX-License-Identifier: BSL-1.0
/*!
 * @file
 * @brief  Simple Vulkan display info
 * @author Christoph Haag <christoph.haag@collabora.com>
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <xcb/randr.h>
#include <xcb/xcb.h>

#include <X11/extensions/Xrandr.h>
#include <vulkan/vulkan_xlib_xrandr.h>

static int display_info(VkInstance instance, VkPhysicalDevice *physical_devices,
                        uint32_t num_physical_devices) {
  VkResult result = VK_ERROR_UNKNOWN;

  PFN_vkGetPhysicalDeviceDisplayPropertiesKHR
      _vkGetPhysicalDeviceDisplayPropertiesKHR =
          (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vkGetInstanceProcAddr(
              instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");

  if (!_vkGetPhysicalDeviceDisplayPropertiesKHR) {
    printf("Failed to get vkGetPhysicalDeviceDisplayPropertiesKHR function\n");
    return 1;
  }

  for (uint32_t i = 0; i < num_physical_devices; i++) {
    VkPhysicalDevice physical_device = physical_devices[i];

    uint32_t num_display_props = 0;
    result = _vkGetPhysicalDeviceDisplayPropertiesKHR(physical_device,
                                                      &num_display_props, NULL);
    if (result != VK_SUCCESS) {
      printf("Failed to get number of physical device display properties %d\n",
             i);
      return 1;
    }

    VkDisplayPropertiesKHR *display_props =
        malloc(sizeof(VkDisplayPropertiesKHR) * num_display_props);
    result = _vkGetPhysicalDeviceDisplayPropertiesKHR(
        physical_device, &num_display_props, display_props);
    if (result != VK_SUCCESS) {
      printf("Failed to get physical device display properties %d\n", i);
      return 1;
    }

    printf("physical device %d has %d displays:\n", i, num_display_props);

    for (uint32_t j = 0; j < num_display_props; j++) {
      VkDisplayPropertiesKHR *display_prop = &display_props[j];
      printf("[%d] %s (%p) ", j, display_prop->displayName,
             (void *)display_prop->display);
      printf("%dx%d\n", display_prop->physicalResolution.width,
             display_prop->physicalResolution.height);
    }

    free(display_props);
  }
  return 0;
}

static int display2_info(VkInstance instance,
                         VkPhysicalDevice *physical_devices,
                         uint32_t num_physical_devices) {
  VkResult result = VK_ERROR_UNKNOWN;

  PFN_vkGetPhysicalDeviceDisplayProperties2KHR
      _vkGetPhysicalDeviceDisplayProperties2KHR =
          (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)vkGetInstanceProcAddr(
              instance, "vkGetPhysicalDeviceDisplayProperties2KHR");

  if (!_vkGetPhysicalDeviceDisplayProperties2KHR) {
    printf("Failed to get vkGetPhysicalDeviceDisplayProperties2KHR function\n");
    return 1;
  }

  for (uint32_t i = 0; i < num_physical_devices; i++) {
    VkPhysicalDevice physical_device = physical_devices[i];

    uint32_t num_display_props = 0;
    result = _vkGetPhysicalDeviceDisplayProperties2KHR(
        physical_device, &num_display_props, NULL);
    if (result != VK_SUCCESS) {
      printf("Failed to get number of physical device display properties %d\n",
             i);
      return 1;
    }

    VkDisplayProperties2KHR *display_props =
        malloc(sizeof(VkDisplayProperties2KHR) * num_display_props);

    for (uint32_t i = 0; i < num_display_props; i++) {
      display_props[i].sType = VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR;
      display_props[i].pNext = NULL;
    }

    result = _vkGetPhysicalDeviceDisplayProperties2KHR(
        physical_device, &num_display_props, display_props);
    if (result != VK_SUCCESS) {
      printf("Failed to get physical device display properties %d\n", i);
      return 1;
    }

    printf("physical device %d has %d displays:\n", i, num_display_props);

    for (uint32_t j = 0; j < num_display_props; j++) {
      VkDisplayProperties2KHR *display_prop2 = &display_props[j];
      VkDisplayPropertiesKHR *display_prop = &display_prop2->displayProperties;

      printf("%s (%p) ", display_prop->displayName,
             (void *)display_prop->display);
      printf("%dx%d\n", display_prop->physicalResolution.width,
             display_prop->physicalResolution.height);
    }

    free(display_props);
  }
  return 0;
}

struct comp_window_direct_randr_display
{
  char *name;
  xcb_randr_output_t output;
  xcb_randr_mode_info_t primary_mode;
  VkDisplayKHR display;
};

int main(void)
{
  printf("vkdisplayinfo\n");
  printf("=============\n");

  VkResult result;

  struct {
    bool display;
    bool display2;
    bool surface;
  } exts = {false};

  {
    uint32_t num_supported_exts = 0;
    result =
        vkEnumerateInstanceExtensionProperties(NULL, &num_supported_exts, NULL);
    if (result != VK_SUCCESS) {
      printf("Failed to get number of supported extensions\n");
      return 1;
    }

    VkExtensionProperties *ext_props =
        malloc(sizeof(VkExtensionProperties) * num_supported_exts);
    result = vkEnumerateInstanceExtensionProperties(NULL, &num_supported_exts,
                                                    ext_props);
    if (result != VK_SUCCESS) {
      printf("Failed to get supported extensions\n");
      return 1;
    }

    for (uint32_t i = 0; i < num_supported_exts; i++) {
      if (strcmp(VK_KHR_DISPLAY_EXTENSION_NAME, ext_props[i].extensionName) ==
          0) {
        exts.display = true;
        printf("instance extension %s is supported\n",
               VK_KHR_DISPLAY_EXTENSION_NAME);
      } else if (strcmp(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
                        ext_props[i].extensionName) == 0) {
        exts.display2 = true;
        printf("instance extension %s is supported\n",
               VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
      } else if (strcmp(VK_KHR_SURFACE_EXTENSION_NAME,
                        ext_props[i].extensionName) == 0) {
        exts.surface = true;
        printf("instance extension %s is supported\n",
               VK_KHR_SURFACE_EXTENSION_NAME);
      }
    }
  }

  if (!exts.surface) {
    printf("Instance Extension %s is required\n",
           VK_KHR_SURFACE_EXTENSION_NAME);
    return 1;
  }
  if (!exts.display && !exts.display2) {
    printf("Instance Extension %s or %s is required\n",
           VK_KHR_DISPLAY_EXTENSION_NAME,
           VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
    return 1;
  }

  // VK_KHR_DISPLAY_EXTENSION_NAME and VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME
#define optional_exts 2

  const char *instance_exts[3 + optional_exts] = {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME,
      VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
  };
  uint32_t num_instance_exts = 3;

  if (exts.display) {
    instance_exts[num_instance_exts++] = VK_KHR_DISPLAY_EXTENSION_NAME;
  }
  if (exts.display2) {
    instance_exts[num_instance_exts++] =
        VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME;
  }

  const VkApplicationInfo application_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .apiVersion = VK_API_VERSION_1_0,
      .applicationVersion = 1,
      .engineVersion = 0,
      .pApplicationName = "vkdisplayinfo",
      .pEngineName = NULL,
  };

  VkInstanceCreateInfo instance_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .ppEnabledExtensionNames = (const char **)instance_exts,
      .enabledExtensionCount = num_instance_exts,
      .pApplicationInfo = &application_info,

  };

  VkInstance instance;
  result = vkCreateInstance(&instance_info, NULL, &instance);
  if (result != VK_SUCCESS) {
    printf("Failed to create vulkan instance\n");
    return 1;
  }

  uint32_t num_physical_devices = 0;
  result = vkEnumeratePhysicalDevices(instance, &num_physical_devices, NULL);
  if (result != VK_SUCCESS) {
    printf("Failed to get number of physical devices\n");
    return 1;
  }

  VkPhysicalDevice *physical_devices =
      malloc(sizeof(VkPhysicalDevice) * num_physical_devices);
  result = vkEnumeratePhysicalDevices(instance, &num_physical_devices,
                                      physical_devices);
  if (result != VK_SUCCESS) {
    printf("Failed to get physical devices\n");
    return 1;
  }

  Display *dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    printf("Could not open X display.");
    return 1;
  }

  xcb_connection_t *connection = XGetXCBConnection(dpy);

  // comp_window_direct_randr_init
  {
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(connection));

    xcb_screen_t *screen = NULL;
    uint32_t display_count = 0;
    struct comp_window_direct_randr_display displays[50] = {0};
    while (iter.rem > 0 && display_count == 0) {
      screen = iter.data;

      // comp_window_direct_randr_get_outputs
      {
        xcb_connection_t *connection = XGetXCBConnection(dpy);
        xcb_randr_query_version_cookie_t version_cookie
            = xcb_randr_query_version(connection, XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION);
        xcb_randr_query_version_reply_t *version_reply
            = xcb_randr_query_version_reply(connection, version_cookie, NULL);

        if (version_reply == NULL) {
            printf("Could not get RandR version.\n");
            return 1;
        }

        printf("RandR version %d.%d\n", version_reply->major_version, version_reply->minor_version);

        if (version_reply->major_version < 1 || version_reply->minor_version < 6) {
            printf("RandR version below 1.6.\n");
        }

        free(version_reply);

        xcb_generic_error_t *error = NULL;
        xcb_intern_atom_cookie_t non_desktop_cookie = xcb_intern_atom(connection,
                                                                      1,
                                                                      strlen("non-desktop"),
                                                                      "non-desktop\n");
        xcb_intern_atom_reply_t *non_desktop_reply = xcb_intern_atom_reply(connection,
                                                                           non_desktop_cookie,
                                                                           &error);

        if (error != NULL) {
            free(non_desktop_reply);
            printf("xcb_intern_atom_reply returned error %d\n", error->error_code);
            return 1;
        }

        if (non_desktop_reply == NULL) {
            printf("non-desktop reply NULL\n");
            return 1;
        }

        if (non_desktop_reply->atom == XCB_NONE) {
            free(non_desktop_reply);
            printf("No output has non-desktop property\n");
            return 1;
        }

        xcb_randr_get_screen_resources_cookie_t resources_cookie
            = xcb_randr_get_screen_resources(connection, screen->root);
        xcb_randr_get_screen_resources_reply_t *resources_reply
            = xcb_randr_get_screen_resources_reply(connection, resources_cookie, NULL);
        xcb_randr_output_t *xcb_outputs = xcb_randr_get_screen_resources_outputs(resources_reply);

        int count = xcb_randr_get_screen_resources_outputs_length(resources_reply);
        if (count < 1) {
            printf("failed to retrieve randr outputs\n");
        }

        for (int i = 0; i < count; i++) {
            xcb_randr_get_output_info_cookie_t output_cookie
                = xcb_randr_get_output_info(connection, xcb_outputs[i], XCB_CURRENT_TIME);
            xcb_randr_get_output_info_reply_t *output_reply
                = xcb_randr_get_output_info_reply(connection, output_cookie, NULL);

            // Only outputs with an available mode should be used
            // (it is possible to see 'ghost' outputs with non-desktop=1).
            if (output_reply->num_modes == 0) {
                free(output_reply);
                continue;
            }

            // Find the first output that has the non-desktop property set.
            xcb_randr_get_output_property_cookie_t prop_cookie;
            prop_cookie = xcb_randr_get_output_property(connection,
                                                        xcb_outputs[i],
                                                        non_desktop_reply->atom,
                                                        XCB_ATOM_NONE,
                                                        0,
                                                        4,
                                                        0,
                                                        0);
            xcb_randr_get_output_property_reply_t *prop_reply = NULL;
            prop_reply = xcb_randr_get_output_property_reply(connection, prop_cookie, &error);
            if (error != NULL) {
                printf("xcb_randr_get_output_property_reply "
                       "returned error %d\n",
                       error->error_code);
                free(prop_reply);
                continue;
            }

            if (prop_reply == NULL) {
                printf("property reply == NULL\n");
                free(prop_reply);
                continue;
            }

            if (prop_reply->type != XCB_ATOM_INTEGER || prop_reply->num_items != 1
                || prop_reply->format != 32) {
                printf("Invalid non-desktop reply\n");
                free(prop_reply);
                continue;
            }

            uint8_t non_desktop = *xcb_randr_get_output_property_data(prop_reply);
            if (non_desktop == 1 || true) {
                // append_randr_display
                {
                    xcb_randr_mode_t *output_modes = xcb_randr_get_output_info_modes(output_reply);

                    uint8_t *name = xcb_randr_get_output_info_name(output_reply);
                    int name_len = xcb_randr_get_output_info_name_length(output_reply);

                    int mode_count = xcb_randr_get_output_info_modes_length(output_reply);
                    if (mode_count == 0) {
                        printf("%s does not have any modes "
                               "available. "
                               "Check `xrandr --prop`.\n",
                               name);
                    }

                    xcb_randr_mode_info_t *mode_infos = xcb_randr_get_screen_resources_modes(
                        resources_reply);

                    int n = xcb_randr_get_screen_resources_modes_length(resources_reply);

                    xcb_randr_mode_info_t *mode_info = NULL;
                    for (int i = 0; i < n; i++)
                        if (mode_infos[i].id == output_modes[0])
                            mode_info = &mode_infos[i];

                    if (mode_info == NULL)
                        printf("No mode with id %d found??\n", output_modes[0]);

                    struct comp_window_direct_randr_display d = {
                        .name = malloc(sizeof(char) * (name_len + 1)),
                        .output = xcb_outputs[i],
                        .primary_mode = *mode_info,
                        .display = VK_NULL_HANDLE,
                    };

                    memcpy(d.name, name, name_len);
                    d.name[name_len] = '\0';

                    display_count += 1;

                    {
                        static PFN_vkGetRandROutputDisplayEXT _vkGetRandROutputDisplayEXT = NULL;
                        if (_vkGetRandROutputDisplayEXT == NULL) {
                            _vkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)
                                vkGetInstanceProcAddr(instance, "vkGetRandROutputDisplayEXT");
                        }

                        // TODO: other physical devices?
                        VkResult ret = _vkGetRandROutputDisplayEXT(physical_devices[0],
                                                                   dpy,
                                                                   d.output,
                                                                   &d.display);
                        if (ret != VK_SUCCESS) {
                            printf("vkGetRandROutputDisplayEXT failed: %d\n", ret);
                            return 1;
                        }

                        if (d.display == VK_NULL_HANDLE) {
                            printf("vkGetRandROutputDisplayEXT returned a null display for %s, "
                                   "ignoring...\n",
                                   d.name);
                            continue;
                        }
                    }

                    displays[display_count - 1] = d;
                    printf("randr display #%d: %s with primary mode %dx%d\n",
                           display_count - 1,
                           displays[display_count - 1].name,
                           displays[display_count - 1].primary_mode.width,
                           displays[display_count - 1].primary_mode.height);
                }
            }

            free(prop_reply);
            free(output_reply);
        }

        free(non_desktop_reply);
        free(resources_reply);
      }
      xcb_screen_next(&iter);
    }
  }

  int ret = 0;
  //
  //    if (exts.display2) {
  //      printf("Using vkGetPhysicalDeviceDisplayProperties2KHR\n");
  //      display2_info(instance, physical_devices, num_physical_devices);
  //    } else {
  //      printf("Using vkGetPhysicalDeviceDisplayPropertiesKHR\n");
  //      display_info(instance, physical_devices, num_physical_devices);
  //    }

  free(physical_devices);

  vkDestroyInstance(instance, NULL);

  return ret;
}
