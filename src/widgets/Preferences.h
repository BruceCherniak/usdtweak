#pragma once
#include "ModalDialogs.h"
class Editor;

// Modal dialog for the preference panel
struct PreferencesModalDialog : public ModalDialog {

    PreferencesModalDialog(Editor &editor);

    void Draw() override;

    const char *DialogId() const override { return "Preferences"; }
    Editor &editor;
};
