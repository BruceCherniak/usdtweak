#pragma once
#include "ModalDialogs.h"
class Editor;

 struct PreferencesModalDialog : public ModalDialog {

    PreferencesModalDialog(Editor &editor);

    void Draw() override;

    const char *DialogId() const override { return "Create usd file"; }
    Editor &editor;
};