#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

int main() {
  printf("vkdisplayinfo\n");
  printf("=============\n");

  const char *instance_exts[] = {
      VK_KHR_DISPLAY_EXTENSION_NAME,
      VK_KHR_SURFACE_EXTENSION_NAME,
  };

  const VkApplicationInfo application_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .apiVersion = VK_API_VERSION_1_2,
      .applicationVersion = 1,
      .engineVersion = 0,
      .pApplicationName = "vkdisplayinfo",
      .pEngineName = NULL,
  };

  VkInstanceCreateInfo instance_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .ppEnabledExtensionNames = (const char **)instance_exts,
      .enabledExtensionCount = 2,
      .pApplicationInfo = &application_info,

  };

  VkResult result;
  VkInstance instance;
  result = vkCreateInstance(&instance_info, NULL, &instance);
  if (result != VK_SUCCESS) {
    printf("Failed to create vulkan instance\n");
    return 1;
  }

  PFN_vkGetPhysicalDeviceDisplayPropertiesKHR
      _vkGetPhysicalDeviceDisplayPropertiesKHR =
          (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vkGetInstanceProcAddr(
              instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");

  if (!_vkGetPhysicalDeviceDisplayPropertiesKHR) {
    printf("Failed to get vkGetPhysicalDeviceDisplayPropertiesKHR function\n");
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
      printf("%s (%p) ", display_prop->displayName,
             (void *)display_prop->display);
      printf("%dx%d\n", display_prop->physicalResolution.width,
             display_prop->physicalResolution.height);
    }

    free(display_props);
  }

  free(physical_devices);

  vkDestroyInstance(instance, NULL);
}
