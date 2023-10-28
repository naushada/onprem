#ifndef __zip_hpp__
#define __zip_hpp__

#define ZIP_DISABLE_DEPRECATED 
#include <zip.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <cstddef>

class Zip {
    public:
        Zip(const std::string& zipFileName): m_handle(zip_open(zipFileName.c_str(), ZIP_RDONLY, &m_err), &zip_close) {}
        ~Zip() = default;
        std::int32_t unzip();
    private:
        std::unique_ptr<zip_t, decltype(&zip_close)> m_handle;
        std::int32_t m_err;
};

#endif