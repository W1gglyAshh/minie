/* Copyright (C) 2025 Gabriel LU - Licensed under GPL v2 */

#include "platform/platform.hpp"

#ifdef _WIN32

#include "platform/platform_win.hpp"

#elif defined(__APPLE__) || defined(__unix__)

#include "platform/platform_unix.hpp"

#endif

std::unique_ptr<Platform> Platform::createPl()
{
#ifdef _WIN32
    return std::make_unique<WinPl>();
#elif defined(__APPLE__) || defined(__unix__)
    return std::make_unique<UnixPl>();
#else
#error "Unsupported platform!"
    return nullptr;
#endif
}
