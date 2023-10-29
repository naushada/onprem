#ifndef __zip_cc__
#define __zip_cc__

#include "zip.hpp"

std::int32_t Zip::unzip(const std::string& path) {

    zip_int64_t fcount = -1;
    zip_stat_t sb;
    zip_flags_t flags = ZIP_FL_COMPRESSED | ZIP_FL_UNCHANGED | ZIP_FL_ENC_RAW ;
    int ret_stat = -1;

    fcount = zip_get_num_entries(m_handle.get(), flags);
    std::cout << __FILE__ <<":" << __LINE__<<" fcount: " << fcount << std::endl;
    if(fcount > 0) {

        //std::cout << "line: " << __LINE__ << " number of files: " << fcount << std::endl;
        for(auto idx = 0; idx < fcount; ++idx) {
            std::string name = zip_get_name(m_handle.get(), idx, flags);

            if(name.length()) {
                //std::cout << "line: " << __LINE__ << " file name: " << name << std::endl;

                ret_stat = zip_stat(m_handle.get(), name.c_str(), (ZIP_FL_ENC_RAW| ZIP_FL_ENC_STRICT), &sb);
                if(ret_stat) {
                    std::cout << "line: " << __LINE__ << " zip_stat is failed for fname: " << name << std::endl;
                    return(-1);
                }

                //std::cout << "line: " << __LINE__ << " zip_stat is ret_stat " << ret_stat << " sb.size: " << sb.size << " sb.name:" << sb.name<< std::endl;
                {
                    std::unique_ptr<zip_file_t, decltype(&zip_fclose)> fp(zip_fopen(m_handle.get(), name.c_str(), ZIP_FL_UNCHANGED), &zip_fclose);
    
                    if(fp == nullptr) {
                        std::cout << "line: " << __LINE__ << " zip_fopen failed fname: " << name << std::endl;
                        continue;
                    }

                    zip_uint64_t nbytes = sb.size;
                    if(!sb.size) {
                        /// this must be directory.
                        auto pos = std::string(name).find("/");
                        if(pos != std::string::npos) {
                            /// create the directory
                            std::filesystem::create_directory(path + name);
                        }
                        continue;
                    }

                    std::vector<char> buf(sb.size);
                    zip_int64_t nread = zip_fread(fp.get(), (void *)buf.data(), nbytes);
                        
                    //std::cout << "line: " << __LINE__ << " nread: " << nread << std::endl;

                    if(nread < 0) {
                        std::cout << "line: " << __LINE__ << " zip_read is failed error:" << zip_file_strerror(fp.get())<< std::endl;
                        fp.reset(nullptr);
                        continue;
                    }

                    /// release the ownership now
                    fp.reset(nullptr);
                    /// open the file and write the content into it.
                    std::ofstream ff;
                    ff.open(path + name);
                    std::string ss(buf.begin(), buf.end());

                    ff << ss;
                    ff.close();
                }
            }
        }
    }
    return(0);

}

#if 0
std::int32_t unzip(const std::string& fileName) {
    Zip zzip(fileName);
    zzip.unzip();
}
#endif



#endif