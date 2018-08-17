#ifndef __PLATFORM_ADAPTER_HPP__
#define __PLATFORM_ADAPTER_HPP__

#include <string>

class ModuleAdapter;

enum platform_adapter_t {
    PLATFORM_ADAPTER_UNKNOWN,
    PLATFORM_ADAPTER_STATIC,
};

class PlatformAdapter {
    public:
        virtual ModuleAdapter* get_module_adapter(const std::string& location) = 0;
        PlatformAdapter(){}
    private:
        PlatformAdapter(const PlatformAdapter&){}
        void operator = (const PlatformAdapter&){}
};

#endif
