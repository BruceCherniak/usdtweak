#include "Preferences.h"
#include "Commands.h"
#include "Editor.h"
#include "Gui.h"
#include "ImGuiHelpers.h"
#include "ResourcesLoader.h"
#include <Style.h>
#include <iostream>

PreferencesModalDialog::PreferencesModalDialog(Editor &editor) : editor(editor){};

bool PreferencesModalDialog::needRestart = false;

void PreferencesModalDialog::Draw() {
    static const char *const panels[] = {"General", "Viewport", "Style", "Fonts"};
    static int current_item = 0;
    const ImGuiContext &g = *GImGui;
    if (needRestart) {
        ImGui::TextColored(ImVec4(1.0, 0.2, 0.2, 1.0), "You must restart usdtweak to apply your changes");
    }
    const float heightWithoutCloseButton = RemainingHeight(1 + needRestart); // - g.Style.ItemSpacing.y*2.f;
    ImVec2 prefContentSize(0, heightWithoutCloseButton);
    ImVec2 prefTabSize(g.FontSize * 5, heightWithoutCloseButton);
    if (ImGui::BeginListBox("##PreferencePanels", prefTabSize)) {
        for (int i = 0; i < 4; ++i) {
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
                needRestart = true;
            }
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
    } else if (current_item == 3) {
        if (ImGui::BeginChild("##Fonts", prefContentSize)) {
            ImGui::Text("Select font from your operating system:");
            std::string &font = ResourcesLoader::GetFontRegularPath();
            std::string &fontMono = ResourcesLoader::GetFontMonoPath();
            ImGui::InputTextWithHint("Regular Font", "Select alternative font path", &font);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                needRestart = true;
            }
            ImGui::InputTextWithHint("Mono Font", "Select alternative font path", &fontMono);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                needRestart = true;
            }
            std::string &selectedGlyph = ResourcesLoader::GetGlyphRangeName();
            if (ImGui::BeginCombo("Glyph range", selectedGlyph.c_str())) {
                for (const std::string &glyphRangeName : ResourcesLoader::GetGlyphRangeNames()) {
                    if (ImGui::Selectable(glyphRangeName.c_str())) {
                        selectedGlyph = glyphRangeName;
                        needRestart = true;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::Separator();
            ImGui::Text("Selecting a unicode fonts allow to display a wider range of characters, chinese, japanese, etc.");
            ImGui::Text("You also have to select the glyph range according to the set of glyph you want to display.");
            ImGui::Text("On the downside, having extended fonts will use more graphic memory and result in a slower startup.");
            ImGui::NewLine();
#ifdef __APPLE__
            ImGui::Text("On macOS, a potential font could be: ");
            ImGui::Text("/System/Library/Fonts/Supplemental/Arial Unicode.ttf");
            if (ImGui::Button("Try Unicode font")) {
                font = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
                fontMono = "/System/Library/Fonts/Supplemental/Courier New.ttf";
                needRestart = true;
            }
            ImGui::SameLine();
#endif
            if (ImGui::Button("Reset to default fonts")) {
                if (!font.empty()) {
                    font = "";
                    needRestart = true;
                }
                if (!fontMono.empty()) {
                    fontMono = "";
                    needRestart = true;
                }
                if (selectedGlyph != "Default") {
                    selectedGlyph = "Default";
                    needRestart = true;
                }
            }
            ImGui::EndChild();
        }
    }
    DrawModalButtonClose();
}
