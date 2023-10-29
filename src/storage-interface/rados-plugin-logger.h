#ifndef SRC_LIBRMB_RADOS_PLUGIN_LOGGER_H_
#define SRC_LIBRMB_RADOS_PLUGIN_LOGGER_H_

#ifdef SPD_LOGGING_FRAMEWORK
 #include <spdlog/spdlog.h>
 #include <spdlog/sinks/stdout_color_sinks.h>
 #include <spdlog/sinks/basic_file_sink.h>
#endif

#include <string>
#include <memory>
#include <cstdarg>
#include <sstream>

namespace librmb {

enum LOG_LEVEL {
    DEBUG_LEVEL,
    WARN,
    INFO,
    ERROR
};


class RadosPluginLogger {
  public:
    // Public static member function to access the instance
    static RadosPluginLogger& getInstance() {
        static RadosPluginLogger instance; // Guaranteed to be destroyed at the end of the program
        return instance;
    }
  
    template <typename... Args>
    void log(librmb::LOG_LEVEL level, const char* format, Args&&... args) {              
    #ifdef SPD_LOGGING_FRAMEWORK

        if(!logger) {
            // you need to first call init(...) before using the logger
            return;
        }
        std::string message = fmt::format(format, std::forward<Args>(args)...);
        logger->log(convert(level), message);
        //TODO: may decrease performance
        logger->flush();    
    #endif    
    }
    void init(const std::string &logFilePath, const std::string &loglevel) {
    #ifdef SPD_LOGGING_FRAMEWORK
        if(logger){
            return;
        }
        bool truncate = false;
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, truncate);
        logger = spdlog::stdout_color_mt("dovecot_ceph_plugin");                
        
        logger->sinks().emplace_back(file_sink);
        if(loglevel == "debug"){
            logger->set_level(spdlog::level::debug);
        }
        else if(loglevel == "info"){
            logger->set_level(spdlog::level::info); 
        }
        else if(loglevel == "warn"){   
            logger->set_level(spdlog::level::warn); 
        }
        else if(loglevel == "error"){           
            logger->set_level(spdlog::level::err);
        }
        else {
            logger->set_level(spdlog::level::debug); 
        }
    #endif
    }
  private:
    
    RadosPluginLogger() {
    //    const std::string logFilePath = "/tmp/dovecot_ceph_plugin.log";
      //  init(logFilePath, "debug");        
    }
    #ifdef SPD_LOGGING_FRAMEWORK
   
    spdlog::level::level_enum convert(librmb::LOG_LEVEL level) {
        switch(level) {
            case librmb::LOG_LEVEL::DEBUG_LEVEL:
                return spdlog::level::debug;
            case librmb::LOG_LEVEL::WARN:
                return spdlog::level::warn;
            case librmb::LOG_LEVEL::INFO:
                return spdlog::level::info;
            case librmb::LOG_LEVEL::ERROR:
                return spdlog::level::err;
            default:
                return spdlog::level::debug;
        }
    }
    #endif
   
    RadosPluginLogger(const RadosPluginLogger&) = delete;
    RadosPluginLogger& operator=(const RadosPluginLogger&) = delete;
    #ifdef SPD_LOGGING_FRAMEWORK
    std::shared_ptr<spdlog::logger> logger;
    #endif
};

}  // namespace librmb

#endif  // SRC_LIBRMB_RADOS_PLUGIN_LOGGER_H_
