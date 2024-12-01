#pragma once
#include "EditorSettings.h"

// Load fonts, ini settings, texture and initialise an imgui context.
 class ResourcesLoader {
  public:
    ResourcesLoader();
    ~ResourcesLoader();

    // Return the settings that are going to be stored on disk
    static EditorSettings & GetEditorSettings();

    static int GetApplicationWidth();
    static int GetApplicationHeight();

    // This should not be called during a frame render.
    static void ScaleUI(float scaleValue);

 private:
     static EditorSettings _editorSettings;
     static bool _resourcesLoaded;
};
