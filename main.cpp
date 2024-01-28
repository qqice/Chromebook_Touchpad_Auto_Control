#include <cstdio>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <tchar.h>
#include <shellapi.h>

// 定义系统托盘图标的ID
#define ID_TRAY_APP_ICON    5000
// 定义退出菜单项的ID
#define ID_TRAY_EXIT        3000

// 添加系统托盘图标
void AddTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_APP_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy_s(nid.szTip, "Touchpad Control");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

// 删除系统托盘图标
void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_APP_ICON;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

// 创建右键菜单
HMENU CreateTrayMenu() {
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));
    return hMenu;
}

// 处理系统托盘的消息
void TrayMessage(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    if (wParam != ID_TRAY_APP_ICON)
        return;

    if (lParam == WM_RBUTTONUP) {
        HMENU hMenu = CreateTrayMenu();
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
        PostMessage(hwnd, WM_NULL, 0, 0);
        DestroyMenu(hMenu);
    }
}

#pragma comment(lib, "setupapi.lib")

// 禁用触摸板的函数
void DisableTouchpad() {
    // 获取设备信息集合
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_HIDCLASS, NULL, NULL, DIGCF_PRESENT); // 修改这里
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        OutputDebugString("Failed to get device information set.\n");
        return;
    }

    // 枚举设备信息
    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
        // 获取设备描述
        TCHAR deviceDesc[256];
        if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)deviceDesc, sizeof(deviceDesc), NULL)) {
            // 检查设备是否是触摸板
            if (_tcsstr(deviceDesc, _T("Touchpad")) != NULL) {
                // 禁用设备
                SP_PROPCHANGE_PARAMS propChangeParams;
                propChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
                propChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
                propChangeParams.StateChange = DICS_DISABLE;
                propChangeParams.Scope = DICS_FLAG_GLOBAL;
                propChangeParams.HwProfile = 0;
                if (SetupDiSetClassInstallParams(deviceInfoSet, &deviceInfoData, &propChangeParams.ClassInstallHeader, sizeof(propChangeParams))
                    && SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, deviceInfoSet, &deviceInfoData)) {
                    OutputDebugString("Disabled touchpad\n");
                } else {
                    OutputDebugString("Failed to disable touchpad\n");
                }
                break;
            }
        }
    }

    // 清理
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
}

// 启用触摸板的函数
void EnableTouchpad() {
    // 获取设备信息集合
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_HIDCLASS, NULL, NULL, DIGCF_PRESENT); // 修改这里
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        OutputDebugString("Failed to get device information set.\n");
        return;
    }

    // 枚举设备信息
    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
        // 获取设备描述
        TCHAR deviceDesc[256];
        if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)deviceDesc, sizeof(deviceDesc), NULL)) {
            // 检查设备是否是触摸板
            if (_tcsstr(deviceDesc, _T("Touchpad")) != NULL) {
                // 启用设备
                SP_PROPCHANGE_PARAMS propChangeParams;
                propChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
                propChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
                propChangeParams.StateChange = DICS_ENABLE;
                propChangeParams.Scope = DICS_FLAG_GLOBAL;
                propChangeParams.HwProfile = 0;
                if (SetupDiSetClassInstallParams(deviceInfoSet, &deviceInfoData, &propChangeParams.ClassInstallHeader, sizeof(propChangeParams))
                    && SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, deviceInfoSet, &deviceInfoData)) {
                    OutputDebugString("Enabled touchpad\n");
                } else {
                    OutputDebugString("Failed to enable touchpad\n");
                }
                break;
            }
        }
    }

    // 清理
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
}

// 检测Windows是否进入平板模式的函数
bool IsTabletMode() {
    // 这里填写检测Windows是否进入平板模式的代码
    return (GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0);
}

// Windows消息处理函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool isTabletMode = IsTabletMode(); // 初始化为当前模式
    switch (uMsg) {
        case WM_SETTINGCHANGE:
        {
            bool newMode = IsTabletMode();
            if (newMode != isTabletMode) { // 只有在模式改变时才执行操作
                isTabletMode = newMode;
                if (isTabletMode) {
                    DisableTouchpad();
                    OutputDebugString("change to tablet mode\n");
                }
                else {
                    EnableTouchpad();
                    OutputDebugString("change to desktop mode\n");
                }
            }
        }
        break;
        case WM_DESTROY:
            RemoveTrayIcon(hwnd);
        PostQuitMessage(0);
        return 0;
        case WM_APP:
            TrayMessage(hwnd, wParam, lParam);
        break;
        case WM_COMMAND:
            if (wParam == ID_TRAY_EXIT) {
                DestroyWindow(hwnd);
            }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Sample Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // 添加系统托盘图标
    AddTrayIcon(hwnd);

    // 隐藏窗口
    ShowWindow(hwnd, SW_HIDE);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}