#include "Include/Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

void Logger::init()
{
  spdlog::set_pattern("%^[%T] %n: %v%$");
  s_CoreLogger = spdlog::stdout_color_mt("GEngine");
  s_CoreLogger->set_level(spdlog::level::trace);

  s_ClientLogger = spdlog::stdout_color_mt("App");
  s_ClientLogger->set_level(spdlog::level::trace);
}
