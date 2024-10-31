#include "WindowInverter.h"

#include <hyprland/src/SharedDefs.hpp>
#include <hyprlang.hpp>

inline HANDLE PHANDLE = nullptr;

inline WindowInverter g_WindowInverter;
inline std::mutex g_InverterMutex;

inline std::vector<SP<HOOK_CALLBACK_FN>> g_Callbacks;

// TODO remove deprecated
inline std::vector<SWindowRule> g_WindowRulesBuildup;
static void addDeprecatedEventListeners();


APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle)
{
    PHANDLE = handle;

    {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.Init();
        g_pConfigManager->m_bForceReload = true;
    }

    g_Callbacks = {};

    addDeprecatedEventListeners();

    HyprlandAPI::addConfigKeyword(
    handle, "chromakey_background",
    [](const char* cmd, const char* val) -> Hyprlang::CParseResult {
        std::vector<std::array<GLfloat, 3>> colors;  // Stores multiple colors
        std::stringstream ss(val);
        std::string component;
        
        while (std::getline(ss, component, ';')) { // Semicolon separates colors
            std::stringstream colorStream(component);
            std::string value;
            std::array<GLfloat, 3> color;

            for (int i = 0; i < 3 && std::getline(colorStream, value, ','); i++) {
                color[i] = std::stof(value);
            }
            colors.push_back(color);
        }
        g_WindowInverter.SetBackground(colors);  // Pass the vector of colors

        return Hyprlang::CParseResult();
    },
    { .allowFlags = false }
    );


    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "render",
        [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        eRenderStage renderStage = std::any_cast<eRenderStage>(data);

        if (renderStage == eRenderStage::RENDER_PRE_WINDOW)
            g_WindowInverter.OnRenderWindowPre();
        if (renderStage == eRenderStage::RENDER_POST_WINDOW)
            g_WindowInverter.OnRenderWindowPost();
    }
    ));
    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "configReloaded",
        [&](void* self, SCallbackInfo&, std::any data) {
            std::lock_guard<std::mutex> lock(g_InverterMutex);
            
            // TODO remove deprecated
            g_WindowInverter.SetRules(std::move(g_WindowRulesBuildup));
            g_WindowRulesBuildup = {};

            // TODO add when removing top
            // g_WindowInverter.Reload();
        }
    ));
    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "closeWindow",
        [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.OnWindowClose(std::any_cast<PHLWINDOW>(data));
    }
    ));

    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "windowUpdateRules",
        [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.InvertIfMatches(std::any_cast<PHLWINDOW>(data));
    }
    ));



    HyprlandAPI::addDispatcher(PHANDLE, "togglewindowchromakey", [&](std::string args) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.ToggleInvert(g_pCompositor->getWindowByRegex(args));
    });
    HyprlandAPI::addDispatcher(PHANDLE, "togglechromakey", [&](std::string args) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.ToggleInvert(g_pCompositor->m_pLastWindow.lock());
    });

    return {
        "hyprchroma",
        "Applies ChromaKey algorithm to windows for transparency effect",
        "alexhulbert",
        "1.0.0"
    };
}

// TODO remove deprecated
Hyprlang::CParseResult onInvertKeyword(const char* COMMAND, const char* VALUE)
{
    Hyprlang::CParseResult res;
    try
    {
        g_WindowRulesBuildup.push_back(ParseRule(VALUE));
    }
    catch (const std::exception& ex)
    {
        res.setError(ex.what());
    }
    return res;
}

// TODO remove deprecated
static void addDeprecatedEventListeners()
{
    HyprlandAPI::addConfigKeyword(
        PHANDLE, "chromakey_enable",
        onInvertKeyword,
        { .allowFlags = false }
    );
}

APICALL EXPORT void PLUGIN_EXIT()
{
    std::lock_guard<std::mutex> lock(g_InverterMutex);
    g_Callbacks = {};
    g_WindowInverter.Unload();
}

APICALL EXPORT std::string PLUGIN_API_VERSION()
{
    return HYPRLAND_API_VERSION;
}
