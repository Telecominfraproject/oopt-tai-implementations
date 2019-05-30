#ifndef __PLATFORM_ADAPTER_HPP__
#define __PLATFORM_ADAPTER_HPP__

#include <string>
#include <unordered_set>

class ModuleAdapter;

enum platform_adapter_t {
    PLATFORM_ADAPTER_UNKNOWN,
    PLATFORM_ADAPTER_STATIC,
};

class PlatformAdapter {
    public:
        virtual ModuleAdapter* get_module_adapter(const std::string& location) = 0;

        /** @brief return the set of loaded module adapters. */
        virtual const std::unordered_set<ModuleAdapter * > list_module_adapters() = 0;
        PlatformAdapter(){}
    private:
        PlatformAdapter(const PlatformAdapter&){}
        void operator = (const PlatformAdapter&){}
};

#endif
