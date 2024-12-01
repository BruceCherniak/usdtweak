#include "StageLayerEditor.h"
#include "SdfLayerEditor.h"
#include "Gui.h"
#include "Constants.h"
#include "ImGuiHelpers.h"
#include "TableLayouts.h"
#include "Commands.h"
#include "FileBrowser.h"
#include "ModalDialogs.h"


static void DrawSublayerTreeNodePopupMenu(const SdfLayerRefPtr &layer, const SdfLayerRefPtr &parent, const std::string &layerPath,
                                          const UsdStageRefPtr &stage) {
    if (ImGui::BeginPopupContextItem()) {
        // Creating anonymous layers means that we need a structure to hold the layers alive as only the path is stored
        // Testing with a static showed that using the identifier as the path works, but it also means more data handling,
        // making sure that the layers are saved before closing, having visual clew for anonymous layers, etc.
        // if (layer && ImGui::MenuItem("Add anonymous sublayer")) {
        //    static auto sublayer = SdfLayer::CreateAnonymous("NewLayer");
        //    ExecuteAfterDraw(&SdfLayer::InsertSubLayerPath, layer, sublayer->GetIdentifier(), 0);
        //}
        if (layer && ImGui::MenuItem("Add sublayer")) {
            DrawSublayerPathEditDialog(layer, "");
        }
        if (parent && !layerPath.empty()) {
            if (ImGui::MenuItem("Remove sublayer")) {
                ExecuteAfterDraw<LayerRemoveSubLayer>(parent, layerPath);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Move up")) {
                ExecuteAfterDraw<LayerMoveSubLayer>(parent, layerPath, true);
            }
            if (ImGui::MenuItem("Move down")) {
                ExecuteAfterDraw<LayerMoveSubLayer>(parent, layerPath, false);
            }
            ImGui::Separator();
        }
        if (layer) {
            if (ImGui::MenuItem("Set edit target")) {
                ExecuteAfterDraw<EditorSetEditTarget>(stage, UsdEditTarget(layer));
            }
            ImGui::Separator();
            DrawLayerActionPopupMenu(layer);
        }
        ImGui::EndPopup();
    }
}

static void DrawLayerSublayerTreeNodeButtons(const SdfLayerRefPtr &layer, const SdfLayerRefPtr &parent,
                                             const std::string &layerPath, const UsdStageRefPtr &stage, int nodeID) {
    if (!layer)
        return;

    ScopedStyleColor transparentButtons(ImGuiCol_Button, ImVec4(ColorTransparent));
    ImGui::BeginDisabled(!layer || !layer->IsDirty() || layer->IsAnonymous());
    if (ImGui::Button(ICON_FA_SAVE)) {
        ExecuteAfterDraw(&SdfLayer::Save, layer, false);
    }
    // TODO it would be useful to have an option to save all the modified children
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!parent);
    if (ImGui::Button(ICON_FA_TRASH)) {
        ExecuteAfterDraw<LayerRemoveSubLayer>(parent, layerPath);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (layer->IsMuted() && ImGui::Button(ICON_FA_VOLUME_MUTE)) {
        ExecuteAfterDraw<LayerUnmute>(layer);
    }

    if (!layer->IsMuted() && ImGui::Button(ICON_FA_VOLUME_OFF)) {
        ExecuteAfterDraw<LayerMute>(layer);
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(!parent||nodeID == 0);

    if (ImGui::Button(ICON_FA_ARROW_UP)) {
        ExecuteAfterDraw<LayerMoveSubLayer>(parent, layerPath, true);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!parent || nodeID == parent->GetNumSubLayerPaths() - 1);
    if (ImGui::Button(ICON_FA_ARROW_DOWN)) {
        ExecuteAfterDraw<LayerMoveSubLayer>(parent, layerPath, false);
    }
    ImGui::EndDisabled();
}

static void DrawLayerSublayerTree(SdfLayerRefPtr layer, SdfLayerRefPtr parent, std::string layerPath, const UsdStageRefPtr &stage,
                                  int nodeID = 0) {
    // Note: layer can be null if it wasn't found
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    if (!layer || !layer->GetNumSubLayerPaths()) {
        treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }
    ImGui::PushID(nodeID);

    bool unfolded = false;
    std::string label =
        layer ? (layer->IsAnonymous() ? std::string(ICON_FA_ATOM " ") : std::string(ICON_FA_FILE " ")) + layer->GetDisplayName()
              : "Not found " + layerPath;
    {
        ScopedStyleColor color(ImGuiCol_Text,
                               layer ? (layer->IsMuted() ? ImVec4(0.5, 0.5, 0.5, 1.0) : ImGui::GetStyleColorVec4(ImGuiCol_Text))
                                     : ImVec4(1.0, 0.2, 0.2, 1.0));
        unfolded = ImGui::TreeNodeEx(label.c_str(), treeNodeFlags);
    }
    if (ImGui::IsItemClicked()) {
        if (ImGui::IsMouseDoubleClicked(0)) {
            ExecuteAfterDraw<EditorFindOrOpenLayer>(layer->GetIdentifier());
        }
    }
    DrawSublayerTreeNodePopupMenu(layer, parent, layerPath, stage);

    ImGui::TableSetColumnIndex(1);
    DrawLayerSublayerTreeNodeButtons(layer, parent, layerPath, stage, nodeID);

    if (unfolded) {
        if (layer) {
            std::vector<std::string> subLayers = layer->GetSubLayerPaths();
            for (int layerId = 0; layerId < subLayers.size(); ++layerId) {
                const std::string &subLayerPath = subLayers[layerId];
                auto subLayer = SdfLayer::FindOrOpenRelativeToLayer(layer, subLayerPath);
                if (!subLayer) { // Try for anonymous layers
                    subLayer = SdfLayer::FindOrOpen(subLayerPath);
                }
                DrawLayerSublayerTree(subLayer, layer, subLayerPath, stage, layerId);
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

struct SublayerRow {
    static constexpr const char *fieldName = "";
};

template <> inline void DrawThirdColumn<SublayerRow>(const int rowId, const UsdStageRefPtr &stage, const SdfLayerHandle &layer) {
    ImGui::PushID(rowId);
    if (ImGui::Selectable(layer->GetIdentifier().c_str())) {
        ExecuteAfterDraw(&UsdStage::SetEditTarget, stage, UsdEditTarget(layer));
    }
    ImGui::PopID();
}


void DrawStageLayerEditor(UsdStageRefPtr stage) {
    if (!stage)
        return;

    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("##DrawLayerSublayers", 2, tableFlags)) {
        ImGui::TableSetupColumn("Stage sublayers", ImGuiTableColumnFlags_WidthStretch);
        // TODO make it relative to font size
        ImGuiContext& g = *GImGui;
        const float buttonsColumnSize = g.FontSize*8.f; // heuristic for 5 buttons + padding and spacing
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, buttonsColumnSize);
        ImGui::TableHeadersRow();
        ImGui::PushID(0);
        DrawLayerSublayerTree(stage->GetSessionLayer(), SdfLayerRefPtr(), std::string(), stage, 0);
        ImGui::PopID();
        ImGui::PushID(1);
        DrawLayerSublayerTree(stage->GetRootLayer(), SdfLayerRefPtr(), std::string(), stage, 0);
        ImGui::PopID();
        ImGui::EndTable();
    }
}
