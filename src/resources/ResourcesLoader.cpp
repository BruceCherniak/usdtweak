#include <iostream>

#include "DefaultImGuiIni.h"
#include "Gui.h"
#include "ResourcesLoader.h"
#include "Constants.h"
#include "Style.h"

// Fonts
#include "FontAwesomeFree5.h"
#include "IBMPlexMonoFree.h"
#include "IBMPlexSansMediumFree.h"

#define GUI_CONFIG_FILE "usdtweak_gui.ini"

#ifdef _WIN64
#include <codecvt>
#include <locale>
#include <shlobj_core.h>
#include <shlwapi.h>
#include <sstream>
#include <winerror.h>

std::string GetConfigFilePath() {
    PWSTR localAppDataDir = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &localAppDataDir))) {
        std::wstringstream configFilePath;
        configFilePath << localAppDataDir << L"\\" GUI_CONFIG_FILE;
        CoTaskMemFree(localAppDataDir);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>
            converter; // TODO: this is deprecated in C++17, find another solution
        return converter.to_bytes(configFilePath.str());
    }
    return GUI_CONFIG_FILE;
}
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
std::string GetConfigFilePath() {
    std::string configPath;
    const char *home = getenv("HOME");
    if (!home) {
        uid_t uid = getuid();
        auto *pwd = getpwuid(uid);
        if (pwd) {
            home = pwd->pw_dir;
        }
    }
    if (home) {
        configPath += home;
#if defined(__APPLE__)
        configPath += "/Library/Preferences/";
#else
        configPath += "/."; // hide the ini in the home dir
#endif
    }
    configPath += GUI_CONFIG_FILE;
    return configPath;
}

#else // Not unix and not windows64

std::string GetConfigFilePath() { return GUI_CONFIG_FILE; }

#endif

static void *UsdTweakDataReadOpen(ImGuiContext *, ImGuiSettingsHandler *iniHandler, const char *name) {
    ResourcesLoader *loader = static_cast<ResourcesLoader *>(iniHandler->UserData);
    return loader;
}


static void UsdTweakDataReadLine(ImGuiContext *, ImGuiSettingsHandler *iniHandler, void *loaderPtr, const char *line) {
    // Loader could be retrieved with
    // ResourcesLoader *loader = static_cast<ResourcesLoader *>(loaderPtr);
    auto &editorSettings = ResourcesLoader::GetEditorSettings();
    editorSettings.ParseLine(line);
    auto &viewportSettings = ResourcesLoader::GetViewportSettings();
    viewportSettings.ParseLine(line);
    // Application
    char strBuffer[1024];
    if (sscanf(line, "Font=%s", strBuffer) == 1) {
        ResourcesLoader::GetFontRegularPath() = strBuffer;
    } else if (sscanf(line, "FontMono=%s", strBuffer) == 1) {
        ResourcesLoader::GetFontMonoPath() = strBuffer;
    }
}

static void UsdTweakDataWriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *iniHandler, ImGuiTextBuffer *buf) {
    // Saving the editor settings
    auto &editorSettings = ResourcesLoader::GetEditorSettings();
    buf->reserve(4096);
    buf->appendf("[%s][%s]\n", iniHandler->TypeName, "Application");
    const std::string& fontRegularPath = ResourcesLoader::GetFontRegularPath();
    if (!fontRegularPath.empty()) {
        buf->appendf("Font=%s\n", fontRegularPath.c_str());
    }
    const std::string& fontMonoPath = ResourcesLoader::GetFontMonoPath();
    if (!fontMonoPath.empty()) {
        buf->appendf("FontMono=%s\n", fontMonoPath.c_str());
    }

    buf->appendf("[%s][%s]\n", iniHandler->TypeName, "Editor");
    editorSettings.Dump(buf);

    auto &viewportSettings = ResourcesLoader::GetViewportSettings();
    buf->appendf("[%s][%s]\n", iniHandler->TypeName, "Viewport");
    viewportSettings.Dump(buf);

}

bool ResourcesLoader::_resourcesLoaded = false;
std::string ResourcesLoader::_font = std::string();
std::string ResourcesLoader::_fontMono = std::string();

EditorSettings ResourcesLoader::_editorSettings = EditorSettings();
EditorSettings &ResourcesLoader::GetEditorSettings() { return _editorSettings; }

ViewportSettings ResourcesLoader::_viewportSettings = ViewportSettings();
ViewportSettings &ResourcesLoader::GetViewportSettings() { return _viewportSettings; }

ResourcesLoader::ResourcesLoader() {
    // There should be only one object of this class, we make sure the constructor is only called once
    if (_resourcesLoaded) {
        std::cerr << "Coding error, ResourcesLoader is called twice" << std::endl;
        exit(-2);
    } else {
        _resourcesLoaded = true;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); // TODO: I am not sure this is a good idea to create an imgui context without windows, double check
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Install handlers to read and write the settings
    ImGuiContext *imGuiContext = ImGui::GetCurrentContext();
    ImGuiSettingsHandler iniHandler;
    iniHandler.TypeName = "UsdTweakData";
    iniHandler.TypeHash = ImHashStr("UsdTweakData");
    iniHandler.ReadOpenFn = UsdTweakDataReadOpen;
    iniHandler.ReadLineFn = UsdTweakDataReadLine;
    iniHandler.WriteAllFn = UsdTweakDataWriteAll;
    iniHandler.UserData = this;
    imGuiContext->SettingsHandlers.push_back(iniHandler);

    ApplyDarkUTStyle();
    
    // Ini file
    // The first time the application is open, there is no default ini and the UI is all over the place.
    // This bit of code adds a default configuration
    ImFileHandle f;
    const std::string configFilePath = GetConfigFilePath();
    std::cout << "Settings: " << configFilePath << std::endl;
    if ((f = ImFileOpen(configFilePath.c_str(), "rb")) == nullptr) {
        ImGui::LoadIniSettingsFromMemory(imgui, 0);
    } else {
        ImFileClose(f);
        ImGui::LoadIniSettingsFromDisk(configFilePath.c_str());
    }
    ScaleUI(GetEditorSettings()._uiScale);
}

int ResourcesLoader :: GetApplicationWidth() { return ResourcesLoader::GetEditorSettings()._mainWindowWidth; }
int ResourcesLoader :: GetApplicationHeight() { return ResourcesLoader::GetEditorSettings()._mainWindowHeight; }

void ResourcesLoader::ScaleUI(float scaleValue) {

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigErrorRecoveryEnableTooltip = false;
    // We don't support the fonts hot reload yet, it is being worked on this PR:
    // https://github.com/ocornut/imgui/pull/3761
    if (io.Fonts->Fonts.Size == 0) {
        ImGui::GetStyle().ScaleAllSizes(scaleValue);
        // Fonts

        ImFontConfig fontConfig;

        // Load primary font
        if (_font.empty() 
        || !io.Fonts->AddFontFromFileTTF(_font.c_str(), 16.f*scaleValue, &fontConfig, io.Fonts->GetGlyphRangesJapanese())) {
            io.Fonts->AddFontFromMemoryCompressedTTF(ibmplexsansmediumfree_compressed_data, ibmplexsansmediumfree_compressed_size, 16.f*scaleValue, &fontConfig, nullptr);
        }

        // Merge Icons in primary font
        static const ImWchar iconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ImFontConfig iconsConfig;
        iconsConfig.MergeMode = true;
        iconsConfig.PixelSnapH = true;
        io.Fonts->AddFontFromMemoryCompressedTTF(fontawesomefree5_compressed_data, fontawesomefree5_compressed_size, 13.f*scaleValue, &iconsConfig, iconRanges); // TODO: size 13 ? or 16 ? like the other fonts ?

        // Monospace font, for the editor
        if (_fontMono.empty() 
        || !io.Fonts->AddFontFromFileTTF(_fontMono.c_str(), 16.f*scaleValue, &fontConfig, io.Fonts->GetGlyphRangesJapanese())) {
            io.Fonts->AddFontFromMemoryCompressedTTF(ibmplexmonofree_compressed_data, ibmplexmonofree_compressed_size, 16.f*scaleValue, nullptr,
                                            nullptr);
        }
     }
}

ResourcesLoader::~ResourcesLoader() {
    // Save the configuration file when the application closes the resources
    const std::string configFilePath = GetConfigFilePath();
    ImGui::SaveIniSettingsToDisk(configFilePath.c_str());
    ImGui::DestroyContext();
}
