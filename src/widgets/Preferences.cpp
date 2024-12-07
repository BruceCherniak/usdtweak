#include "Preferences.h"
#include "Commands.h"
#include "Editor.h"
#include "Gui.h"
#include "ImGuiHelpers.h"
#include "ResourcesLoader.h"
#include <iostream>
#include <Style.h>

PreferencesModalDialog::PreferencesModalDialog(Editor &editor) : editor(editor){};

void PreferencesModalDialog::Draw() {
    static const char *const panels[] = {"General", "Viewport", "Style"};
    static int current_item = 0;
    const ImGuiContext &g = *GImGui;
    const float heightWithoutCloseButton = RemainingHeight(1); // - g.Style.ItemSpacing.y*2.f;
    ImVec2 prefContentSize(0, heightWithoutCloseButton);
    ImVec2 prefTabSize(g.FontSize * 5, heightWithoutCloseButton);
    if (ImGui::BeginListBox("##PreferencePanels", prefTabSize)) {
        for (int i = 0; i < 3; ++i) {
            if (ImGui::Selectable(panels[i], i == current_item)) {
                current_item = i;
            }
        }
        ImGui::EndListBox();
    }

    ImGui::SameLine();
    if (current_item == 0) {
        if (ImGui::BeginChild("##General", prefContentSize)) {
            float uiScale = editor.GetScaleUI();
            if (ImGui::SliderFloat("UI scaling", &uiScale, 0.5f, 3.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat)) {
                ExecuteAfterDraw<EditorScaleUI>(uiScale);
            }
            ImGui::Text("You must restart usdtweak to apply the new uiscale");
            ImGui::EndChild();
        }
    } else if (current_item == 1) {
        if (ImGui::BeginChild("##Viewport", prefContentSize)) {
            ViewportSettings &viewportSettings = ResourcesLoader::GetViewportSettings();
            ImGui::Checkbox("Texture On by default", &viewportSettings._useMaterials);
            ImGui::EndChild();
        }

    } else if (current_item == 2) {
        if (ImGui::BeginChild("##Style", prefContentSize)) {
            ShowStyleEditor(nullptr);
            ImGui::EndChild();
        }
    }
    DrawModalButtonClose();
}
