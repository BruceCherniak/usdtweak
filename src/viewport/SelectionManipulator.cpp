#include <pxr/usd/kind/registry.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/prim.h>
#include "Viewport.h"
#include "SelectionManipulator.h"
#include "Gui.h"
#include "Commands.h"

bool SelectionManipulator::IsPickablePath(const UsdStage &stage, const SdfPath &path) {
    auto prim = stage.GetPrimAtPath(path);
    if (prim.IsPseudoRoot())
        return true;
    if (GetPickMode() == SelectionManipulator::PickMode::Prim)
        return true;

    TfToken primKind;
    UsdModelAPI(prim).GetKind(&primKind);
    if (GetPickMode() == SelectionManipulator::PickMode::Model && KindRegistry::GetInstance().IsA(primKind, KindTokens->model)) {
        return true;
    }
    if (GetPickMode() == SelectionManipulator::PickMode::Assembly &&
        KindRegistry::GetInstance().IsA(primKind, KindTokens->assembly)) {
        return true;
    }

    // Other possible tokens
    // KindTokens->component
    // KindTokens->group
    // KindTokens->subcomponent

    // We can also test for xformable or other schema API

    return false;
}

Manipulator *SelectionManipulator::OnUpdate(Viewport &viewport) {
    Selection &selection = viewport.GetSelection();
    auto mousePosition = viewport.GetMousePosition();
    SdfPath outHitPrimPath;
    SdfPath outHitInstancerPath;
    int outHitInstanceIndex = 0;
    viewport.TestIntersection(mousePosition, outHitPrimPath, outHitInstancerPath, outHitInstanceIndex);
    if (!outHitPrimPath.IsEmpty()) {
        if (viewport.GetCurrentStage()) {
            while (!IsPickablePath(*viewport.GetCurrentStage(), outHitPrimPath)) {
                outHitPrimPath = outHitPrimPath.GetParentPath();
            }
        }

        if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
            // TODO: a command
            selection.AddSelected(viewport.GetCurrentStage(), outHitPrimPath);
        } else {
            ExecuteAfterDraw<EditorSetSelection>(viewport.GetCurrentStage(), outHitPrimPath);
        }
    } else if (outHitInstancerPath.IsEmpty()) {
        selection.Clear(viewport.GetCurrentStage());
    }
    return viewport.GetManipulator<MouseHoverManipulator>();
}

void SelectionManipulator::OnDrawFrame(const Viewport &) {
    // Draw a rectangle for the selection
}

void DrawPickMode(SelectionManipulator &manipulator) {
    static const char *PickModeStr[3] = {ICON_FA_HAND_POINTER "  P", ICON_FA_HAND_POINTER "  M", ICON_FA_HAND_POINTER "  A"};
    static const char *PickModeSelec[3] = {"Prim", "Model","Assembly"};
    ImGui::SetNextItemWidth(35); // TODO compute size for icon + letter
    if (ImGui::BeginCombo("##Pick mode", PickModeStr[int(manipulator.GetPickMode())], ImGuiComboFlags_NoArrowButton)) {
        if (ImGui::Selectable(PickModeSelec[0])) {
            manipulator.SetPickMode(SelectionManipulator::PickMode::Prim);
        }
        if (ImGui::Selectable(PickModeSelec[1])) {
            manipulator.SetPickMode(SelectionManipulator::PickMode::Model);
        }
        if (ImGui::Selectable(PickModeSelec[2])) {
            manipulator.SetPickMode(SelectionManipulator::PickMode::Assembly);
        }
        ImGui::EndCombo();
    }
}
