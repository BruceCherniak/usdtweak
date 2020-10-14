#pragma once
#include <set>
#include <pxr/usd/usd/stageCache.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <Selection.h>
#include <Viewport.h>

class GLFWwindow;

PXR_NAMESPACE_USING_DIRECTIVE

/// Editor contains the data shared between widgets, like selections, stages, etc etc
class Editor {

public:
    Editor();
    ~Editor();

    /// Removing the copy constructors as we want to make sure there are no unwanted copies of the
    /// editor. There should be only one editor for now but we want to control the construction
    /// and destruction of the editor to delete properly the contexts, so it's not a singleton
    Editor(const Editor &) = delete;
    Editor &operator=(const Editor &) = delete;

    /// Setting _shutdownRequested to true will stop the main loop
    bool ShutdownRequested() const { return _shutdownRequested; }

    /// Sets the current edited layer
    void SetCurrentLayer(SdfLayerRefPtr layer);
    SdfLayerRefPtr GetCurrentLayer() { return _currentLayer; }

    /// List of stages
    /// Using a stage cache to store the stages, seems to work well
    UsdStageRefPtr GetCurrentStage() { return _currentStage; }
    void SetCurrentStage(UsdStageCache::Id current);
    void SetCurrentStage(UsdStageRefPtr stage);

    UsdStageCache & GetStageCache() { return _stageCache; }

    /// Returns the selected primspec
    /// There should be one selected primspec per layer ideally, so it's very likely this function will move
    SdfPrimSpecHandle &GetSelectedPrimSpec() { return _selectedPrimSpec; }

    /// Create a new layer in file path
    void CreateLayer(const std::string &path);
    void ImportLayer(const std::string &path);
    void CreateStage(const std::string &path);
    void ImportStage(const std::string &path);
    void SaveCurrentLayerAs(const std::string &path);

    /// Render the hydra viewport
    void HydraRender();

    ///
    /// Drawing functions for the main editor
    ///

    /// Draw the menu bar
    void DrawMainMenuBar();

    /// Draw the UI
    void Draw();

    /// Handle drag and drop from external applications
    static void DropCallback(GLFWwindow *window, int count, const char **paths);

    /// There is only one viewport for now, but could have multiple in the future
    Viewport &GetViewport();

private:

    /// Using a stage cache to store the stages, seems to work well
    UsdStageCache _stageCache;

        /// List of layers.
    /// NOTE "interface wise" a SdfLayerHandleSet would be better than a std::set<SdfLayerRefPtr>,
    /// but SdfLayerHandleSet doesn't keep the smart pointer live for some reason. (to be investigated)
    // SdfLayerHandleSet _layers;
    std::set<SdfLayerRefPtr> _layers;
    SdfLayerRefPtr _currentLayer;

    /// Setting _shutdownRequested to true will stop the main loop
    bool _shutdownRequested = false;

    ///
    /// Editor manages the windows states
    ///
    bool _showDebugWindow = false;
    bool _showPropertyEditor = false;
    bool _showOutliner = false;
    bool _showTimeline = false;
    bool _showLayerEditor = false;
    bool _showTheater = false;
    bool _showPrimSpecEditor = false;
    bool _showViewport = false;

    UsdStageRefPtr _currentStage;
    Viewport _viewport;

    // Editor owns the selection for the application
    Selection _selection;

    /// Selected prim spec. This variable might move somewhere else
    SdfPrimSpecHandle _selectedPrimSpec;
};
