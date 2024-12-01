#include "Debug.h"
#include "Gui.h"
#include "pxr/base/trace/reporter.h"
#include "pxr/base/trace/trace.h"
#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>
#include <pxr/base/tf/debug.h>
#include <sstream>

PXR_NAMESPACE_USING_DIRECTIVE

static void DrawTraceReporter() {

    static std::string reportStr;

    if (ImGui::Button("Start Tracing")) {
        TraceCollector::GetInstance().SetEnabled(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop Tracing")) {
        TraceCollector::GetInstance().SetEnabled(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset counters")) {
        TraceReporter::GetGlobalReporter()->ClearTree();
    }
    ImGui::SameLine();

    if (ImGui::Button("Update tree")) {
        TraceReporter::GetGlobalReporter()->UpdateTraceTrees();
    }
    if (TraceCollector::IsEnabled()) {
        std::ostringstream report;
        TraceReporter::GetGlobalReporter()->Report(report);
        reportStr = report.str();
    }
    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[1]);
    const ImVec2 size(-FLT_MIN, -10);
    ImGui::InputTextMultiline("##TraceReport", &reportStr, size);
    ImGui::PopFont();
}

static void DrawDebugCodes() {
    // TfDebug::IsCompileTimeEnabled()
    ImVec2 listBoxSize(-FLT_MIN, -10);
    if (ImGui::BeginListBox("##DebugCodes", listBoxSize)) {
        for (auto &code : TfDebug::GetDebugSymbolNames()) {
            bool isEnabled = TfDebug::IsDebugSymbolNameEnabled(code);
            if (ImGui::Checkbox(code.c_str(), &isEnabled)) {
                TfDebug::SetDebugSymbolsByName(code, isEnabled);
            }
        }
        ImGui::EndListBox();
    }
}

static void DrawPlugins() {
    const PlugPluginPtrVector &plugins = PlugRegistry::GetInstance().GetAllPlugins();
    ImVec2 listBoxSize(-FLT_MIN, -10);
    if (ImGui::BeginListBox("##Plugins", listBoxSize)) {
        for (const auto &plug : plugins) {
            const std::string &plugName = plug->GetName();
            const std::string &plugPath = plug->GetPath();
            bool isLoaded = plug->IsLoaded();
            if (ImGui::Checkbox(plugName.c_str(), &isLoaded)) {
                plug->Load(); // There is no Unload in the API
            }
            ImGui::SameLine();
            ImGui::Text("%s", plugPath.c_str());
        }
        ImGui::EndListBox();
    }
}

// Draw a preference like panel
void DrawDebugUI() {
    static const char *const panels[] = {"Timings", "Debug codes", "Trace reporter", "Plugins"};
    static int current_item = 0;
    const ImGuiContext &g = *GImGui;
    ImGui::PushItemWidth(g.FontSize * 7); // heuristic
    ImGui::ListBox("##DebugPanels", &current_item, panels, 4);
    ImGui::SameLine();
    if (current_item == 0) {
        ImGui::BeginChild("##Timing");
        ImGui::Text("ImGui: %.3f ms/frame  (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::EndChild();
    } else if (current_item == 1) {
        ImGui::BeginChild("##DebugCodes");
        DrawDebugCodes();
        ImGui::EndChild();
    } else if (current_item == 2) {
        ImGui::BeginChild("##TraceReporter");
        DrawTraceReporter();
        ImGui::EndChild();
    } else if (current_item == 3) {
        ImGui::BeginChild("##Plugins");
        DrawPlugins();
        ImGui::EndChild();
    }
}
