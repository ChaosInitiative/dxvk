#include "dxvk_device_filter.h"

#include <cstdio>

namespace dxvk {
  
  DxvkDeviceFilter::DxvkDeviceFilter(DxvkDeviceFilterFlags flags)
  : m_flags(flags) {
    m_matchDeviceName = env::getEnvVar("DXVK_FILTER_DEVICE_NAME");
    
    if (m_matchDeviceName.size() != 0)
      m_flags.set(DxvkDeviceFilterFlag::MatchDeviceName);
  }
  
  
  DxvkDeviceFilter::~DxvkDeviceFilter() {
    
  }

  // returns 1.3.0 or whatever the user decides to override us with
  static auto getMinVersion() {
    static const auto  ver = []() {
      auto minVer = env::getEnvVar("DXVK_MIN_VK_VERSION");
      int major = 1, minor = 3, patch = 0;
      if (minVer.length() > 0) {
        int n = std::sscanf(minVer.c_str(), "%d.%d.%d", &major, &minor, &patch);
        major = (n>0) ? major : 1;
        minor = (n>1) ? minor : 3;
        patch = (n>2) ? patch : 0;
      }
      return VK_MAKE_VERSION(major, minor, patch);
    }();
    return ver;
  }
  
  bool DxvkDeviceFilter::testAdapter(const VkPhysicalDeviceProperties& properties) const {
    if (properties.apiVersion < getMinVersion()) {
      Logger::warn(str::format("Skipping Vulkan ",
        VK_VERSION_MAJOR(properties.apiVersion), ".",
        VK_VERSION_MINOR(properties.apiVersion), " adapter: ",
        properties.deviceName));
      return false;
    }

    if (m_flags.test(DxvkDeviceFilterFlag::MatchDeviceName)) {
      if (std::string(properties.deviceName).find(m_matchDeviceName) == std::string::npos)
        return false;
    }

    if (m_flags.test(DxvkDeviceFilterFlag::SkipCpuDevices)) {
      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
        Logger::warn(str::format("Skipping CPU adapter: ", properties.deviceName));
        return false;
      }
    }

    return true;
  }
  
}
