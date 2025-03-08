#include "platform.hpp"

#include <memory>

#ifdef _WIN32

#include "platform_windows.hpp"

#elif defined (__APPLE__) || defined(__unix__)

#include "platform_unix.hpp"

#endif

std::unique_ptr<Platform> Platform::createPlatform()
{
#ifdef _WIN32

    return std::make_unique(new WindowsPlatform());

#elif defined (__APPLE__) || defined (__unix__)

    return std::make_unique(new UnixPlatform());
#else
#error "Unsupported platform!"

    return nullptr;

#endif
}
