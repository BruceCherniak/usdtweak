#pragma once
#include "EditorSettings.h"
#include "ViewportSettings.h"

// Load fonts, ini settings, texture and initialise an imgui context.
// It allows to keep data that have a longer lifetime than the editor or the widgets.
 class ResourcesLoader {
  public:
    ResourcesLoader();
    ~ResourcesLoader();

    // Return the settings that the resource loader has loaded when the application starter.
    // Those settings will also be saved when the application closes.
    // Note that he editor and the viewport might have their own copy of the settings.
    static EditorSettings & GetEditorSettings();
    static ViewportSettings & GetViewportSettings();

    // These should belong to ApplicationSettings
    static int GetApplicationWidth();
    static int GetApplicationHeight();

    static std::string& GetFontRegularPath() { return _font; };
    static std::string& GetFontMonoPath() { return _fontMono; };

    // This should not be called during a frame render.
    static void ScaleUI(float scaleValue);

 private:
     // We keep a static copy of the editor settings because the lifetime
     // of the ResourceLoader is longer than the Editor and we want locality
     // between editor and editor settings.
     static EditorSettings _editorSettings;
     static ViewportSettings _viewportSettings;
     static std::string _font;
     static std::string _fontMono;
     static bool _resourcesLoaded;
};
