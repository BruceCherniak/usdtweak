#include "Timeline.h"
#include "Commands.h"
#include "Gui.h"
#include <iostream>

// The easiest version of a timeline: a slider
void DrawTimeline(UsdStageRefPtr stage, UsdTimeCode &currentTimeCode) {
    const bool hasStage = stage;
    const ImGuiContext& g = *GImGui;
    const float widgetWidth = g.FontSize * 4.f; // heuristic
    int startTime = hasStage ? static_cast<int>(stage->GetStartTimeCode()) : 0;
    int endTime = hasStage ? static_cast<int>(stage->GetEndTimeCode()) : 0;

    // Start time
    ImGui::PushItemWidth(widgetWidth);
    ImGui::InputInt("##Start", &startTime, 0);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        if (hasStage && stage->GetStartTimeCode() != static_cast<double>(startTime)) {
            ExecuteAfterDraw(&UsdStage::SetStartTimeCode, stage, static_cast<double>(startTime));
        }
    }

    // Frame Slider
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetWindowWidth() -
                         5 * widgetWidth - 7 * g.Style.ItemSpacing.x ); // 5 to account for the 5 input widgets and the space between them
    int currentTimeSlider = static_cast<int>(currentTimeCode.GetValue());
    if (ImGui::SliderInt("##SliderFrame", &currentTimeSlider, startTime, endTime)) {
        currentTimeCode = static_cast<UsdTimeCode>(currentTimeSlider);
    }

    // End time
    ImGui::SameLine();
    ImGui::PushItemWidth(widgetWidth);
    ImGui::InputInt("##End", &endTime, 0);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        if (hasStage && stage->GetEndTimeCode() != static_cast<double>(endTime)) {
            ExecuteAfterDraw(&UsdStage::SetEndTimeCode, stage, static_cast<double>(endTime));
        }
    }

    // Frame input
    ImGui::SameLine();
    ImGui::PushItemWidth(widgetWidth);
    double currentTime = currentTimeCode.GetValue();
    ImGui::InputDouble("##Frame", &currentTime);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        if (currentTimeCode.GetValue() != static_cast<double>(currentTime)) {
            currentTimeCode = static_cast<UsdTimeCode>(currentTime);
        }
    }

    // Play button
    ImGui::SameLine();
    ImGui::PushItemWidth(widgetWidth);
    if (ImGui::Button("Play", ImVec2(widgetWidth, 0))) {
        ExecuteAfterDraw<EditorStartPlayback>();
    }

    // Stop button
    ImGui::SameLine();
    ImGui::PushItemWidth(widgetWidth);
    if (ImGui::Button("Stop", ImVec2(widgetWidth, 0))) {
        ExecuteAfterDraw<EditorStopPlayback>();
    }
}
