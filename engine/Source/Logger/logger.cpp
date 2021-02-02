#include "Include/logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

void Logger::Init()
{
  spdlog::set_pattern("%^[%T] %n: %v%$");
  s_CoreLogger = spdlog::stdout_color_mt("Engine");
  s_CoreLogger->set_level(spdlog::level::trace);

  s_ClientLogger = spdlog::stdout_color_mt("Game");
  s_ClientLogger->set_level(spdlog::level::trace);
}
