#pragma once
struct ImGuiTextBuffer;

struct ViewportSettings {
    
    // Default value of the viewport use materials
    bool _useMaterials = false;
    
    // Serialization functions
    void ParseLine(const char *line);
    void Dump(ImGuiTextBuffer *);
};
