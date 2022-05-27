#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

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

int main() {
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

  const char *instance_exts[3] = {
      VK_KHR_SURFACE_EXTENSION_NAME,
  };
  uint32_t num_instance_exts = 1;

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

  int ret = 0;
  if (exts.display2) {
    printf("Using vkGetPhysicalDeviceDisplayProperties2KHR\n");
    display2_info(instance, physical_devices, num_physical_devices);
  } else {
    printf("Using vkGetPhysicalDeviceDisplayPropertiesKHR\n");
    display_info(instance, physical_devices, num_physical_devices);
  }

  free(physical_devices);

  vkDestroyInstance(instance, NULL);

  return ret;
}
