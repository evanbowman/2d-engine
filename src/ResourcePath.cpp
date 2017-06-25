#include "ResourcePath.hpp"

namespace {
// TODO: replace with std::optional after C++17 release
std::pair<std::string, bool> cachedPath;

#if defined(_WIN32) or defined(_WIN64)
#include <Windows.h>

std::string ResourcePathImpl() {
    HMODULE hModule = GetModuleHandleW(nullptr);
    char buffer[MAX_PATH];
    GetModuleFileName(hModule, buffer, MAX_PATH);
    const std::string path(buffer);
    const std::size_t lastFwdSlash = path.find_last_of('\\');
    std::string pathWithoutBinary = path.substr(0, lastFwdSlash + 1);
    return pathWithoutBinary + "..\\..\\res\\";
}

#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc-runtime.h>
#include <objc/objc.h>

std::string ResourcePathImpl() {
    id pool = reinterpret_cast<id>(objc_getClass("NSAutoreleasePool"));
    std::string rpath;
    if (!pool) {
        return rpath;
    }
    pool = objc_msgSend(pool, sel_registerName("alloc"));
    if (!pool) {
        return rpath;
    }
    pool = objc_msgSend(pool, sel_registerName("init"));
    id bundle = objc_msgSend(reinterpret_cast<id>(objc_getClass("NSBundle")),
                             sel_registerName("mainBundle"));
    if (bundle) {
        id path = objc_msgSend(bundle, sel_registerName("resourcePath"));
        rpath = reinterpret_cast<const char *>(
                    objc_msgSend(path, sel_registerName("UTF8String"))) +
            /* Fixme: if ever bundling this project, don't append res/
             * because NSBundle will automatically find the right dir. */
                std::string("/../res/");
    }
    objc_msgSend(pool, sel_registerName("drain"));
    return rpath;
}

#elif __LINUX__
#include <linux/limits.h>
#include <unistd.h>

std::string ResourcePathImpl() {
    char buffer[PATH_MAX];
    [[gnu::unused]] const std::size_t bytesRead =
        readlink("/proc/self/exe", buffer, sizeof(buffer));
    const std::string path(buffer);
    const std::size_t lastFwdSlash = path.find_last_of("/");
    std::string pathWithoutBinary = path.substr(0, lastFwdSlash + 1);
    return pathWithoutBinary + "../res/";
}
#endif
}

const std::string & ResourcePath() {
    if (!cachedPath.second) {
        cachedPath.first = ResourcePathImpl();
        cachedPath.second = true;
    }
    return cachedPath.first;
}