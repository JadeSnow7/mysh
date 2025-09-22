#include "platform.h"

// 包含平台特定实现
#ifdef PLATFORM_LINUX
#include "linux/platform_linux.cpp"
#elif defined(PLATFORM_WINDOWS)
#include "windows/platform_windows.cpp"
#elif defined(PLATFORM_MACOS)
#include "macos/platform_macos.cpp"
#endif

std::unique_ptr<PlatformInterface> createPlatformInterface() {
#ifdef PLATFORM_LINUX
    return std::make_unique<LinuxPlatform>();
#elif defined(PLATFORM_WINDOWS)
    return std::make_unique<WindowsPlatform>();
#elif defined(PLATFORM_MACOS)
    return std::make_unique<MacOSPlatform>();
#else
    #error "Unsupported platform"
    return nullptr;
#endif
}