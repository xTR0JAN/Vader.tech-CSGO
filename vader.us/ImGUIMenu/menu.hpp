#pragma once

#include <string>
#include <imgui.h>

struct IDirect3DDevice9;

extern ImFont* gravity;
extern ImFont* gravityBold;

class IMGUIMenu
{
public:
    bool Initialize(IDirect3DDevice9* pDevice);
    void Shutdown();

    void OnDeviceLost();
    void OnDeviceReset();


    //IDirect3DTexture9* menuLogo;;

    void Render();

    void Toggle();

    bool IsVisible() const { return _visible; }

    bool Opened = true, Initialized = false;
private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;

};

inline IMGUIMenu g_IMGUIMenu;