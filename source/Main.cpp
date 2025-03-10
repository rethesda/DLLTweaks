#include "Hooks.h"
#include "Settings.h"

void InitLogging()
{
    auto path = SKSE::log::log_directory();

    if (!path) { return; }

    const auto plugin = SKSE::PluginDeclaration::GetSingleton();
    *path /= std::format("{}.log", plugin->GetName());

    std::vector<spdlog::sink_ptr> sinks{
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true),
        std::make_shared<spdlog::sinks::msvc_sink_mt>()
    };

    const auto logger = std::make_shared<spdlog::logger>("global", sinks.begin(), sinks.end());

    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(logger));
    spdlog::set_pattern("[%^%L%$] %v");
}

void InitMessaging()
{
    TRACE("Initializing messaging listener.");

    const auto messaging_interface = SKSE::GetMessagingInterface();

    if (!messaging_interface->RegisterListener([](SKSE::MessagingInterface::Message* a_message) {
        switch (a_message->type) {
        case SKSE::MessagingInterface::kInputLoaded:
            {
                Settings::Load();
                Fixes::Install();
                Tweaks::Install();
            }
            break;
        case SKSE::MessagingInterface::kNewGame:
            {
                Fixes::NordRaceStats::Install();
            }
            break;
        }

        })) {
        stl::report_and_fail("Failed to initialize message listener.");
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    InitLogging();

    const auto plugin = SKSE::PluginDeclaration::GetSingleton();

    INFO("{} v{} is loading...", plugin->GetName(), plugin->GetVersion());

    SKSE::Init(a_skse);

    SKSE::AllocTrampoline(14 * 4);

    InitMessaging();

    INFO("{} loaded.", plugin->GetName());

    return true;
}

