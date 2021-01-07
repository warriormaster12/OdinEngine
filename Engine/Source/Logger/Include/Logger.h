#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>



class Logger
{
public:
  static void init();

  inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() {return s_CoreLogger; }
  inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {return s_ClientLogger;}

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

#define GENGINE_CORE_TRACE(...)   ::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define GENGINE_CORE_INFO(...)    ::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define GENGINE_CORE_WARN(...)    ::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define GENGINE_CORE_ERROR(...)   ::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define GENGINE_CORE_FATAL(...)   ::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define GENGINE_TRACE(...)   ::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define GENGINE_INFO(...)    ::Logger::GetClientLogger()->info(__VA_ARGS__)
#define GENGINE_WARN(...)    ::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define GENGINE_ERROR(...)   ::Logger::GetClientLogger()->error(__VA_ARGS__)
#define GENGINE_FATAL(...)   ::Logger::GetClientLogger()->critical(__VA_ARGS__)

#define GENGINE_PRINT(x) std::cout<<x<<std::endl; 
