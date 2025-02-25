/****************************************************************************
 Copyright (c) 2021 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/

#include "platform/win32/WindowsPlatform.h"
#include "sdl2/SDL.h"
#include "sdl2/SDL_main.h"
#include "sdl2/SDL_syswm.h"

#include <MMSystem.h>
#include <Windows.h>
#include <shellapi.h>
#include <sstream>
#include "platform/interfaces/OSInterface.h"
#include "platform/interfaces/modules/ISystemWindow.h"

namespace {
/**
        @brief  This function changes the PVRFrame show/hide setting in register.
        @param  bEnable If true show the PVRFrame window, otherwise hide.
        */
void PVRFrameEnableControlWindow(bool bEnable) {
    HKEY hKey = 0;

    // Open PVRFrame control key, if not exist create it.
    if (ERROR_SUCCESS != RegCreateKeyExW(HKEY_CURRENT_USER,
                                         L"Software\\Imagination Technologies\\PVRVFRame\\STARTUP\\",
                                         0,
                                         0,
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS,
                                         0,
                                         &hKey,
                                         nullptr)) {
        return;
    }

    const WCHAR *wszValue        = L"hide_gui";
    const WCHAR *wszNewData      = (bEnable) ? L"NO" : L"YES";
    WCHAR        wszOldData[256] = {0};
    DWORD        dwSize          = sizeof(wszOldData);
    LSTATUS      status          = RegQueryValueExW(hKey, wszValue, 0, nullptr, (LPBYTE)wszOldData, &dwSize);
    if (ERROR_FILE_NOT_FOUND == status               // the key not exist
        || (ERROR_SUCCESS == status                  // or the hide_gui value is exist
            && 0 != wcscmp(wszNewData, wszOldData))) // but new data and old data not equal
    {
        dwSize = sizeof(WCHAR) * (wcslen(wszNewData) + 1);
        RegSetValueEx(hKey, wszValue, 0, REG_SZ, (const BYTE *)wszNewData, dwSize);
    }

    RegCloseKey(hKey);
}

std::unordered_map<int, cc::KeyCode> gKeyMap = {
    {SDLK_APPLICATION, cc::KeyCode::CONTEXT_MENU},
    {SDLK_SCROLLLOCK, cc::KeyCode::SCROLLLOCK},
    {SDLK_PAUSE, cc::KeyCode::PAUSE},
    {SDLK_PRINTSCREEN, cc::KeyCode::PRINT_SCREEN},
    {SDLK_INSERT, cc::KeyCode::INSERT},
    {SDLK_ESCAPE, cc::KeyCode::ESCAPE},
    {SDLK_MINUS, cc::KeyCode::MINUS},
    {SDLK_LSHIFT, cc::KeyCode::SHIFT_LEFT},
    {SDLK_RSHIFT, cc::KeyCode::SHIFT_RIGHT},
    {SDLK_EQUALS, cc::KeyCode::EQUAL},
    {SDLK_BACKSLASH, cc::KeyCode::BACKSLASH},
    {SDLK_BACKQUOTE, cc::KeyCode::BACKQUOTE},
    {SDLK_BACKSPACE, cc::KeyCode::BACKSPACE},
    {SDLK_RETURN, cc::KeyCode::ENTER},
    {SDLK_RETURN2, cc::KeyCode::ENTER},
    {SDLK_LEFTBRACKET, cc::KeyCode::BRACKET_LEFT},
    {SDLK_RIGHTBRACKET, cc::KeyCode::BRACKET_RIGHT},
    {SDLK_SEMICOLON, cc::KeyCode::SEMICOLON},
    {SDLK_QUOTE, cc::KeyCode::QUOTE},
    {SDLK_TAB, cc::KeyCode::TAB},
    {SDLK_LCTRL, cc::KeyCode::CONTROL_LEFT},
    {SDLK_RCTRL, cc::KeyCode::CONTROL_RIGHT},
    {SDLK_LALT, cc::KeyCode::ALT_LEFT},
    {SDLK_RALT, cc::KeyCode::ALT_RIGHT},
    {SDLK_LEFT, cc::KeyCode::ARROW_LEFT},
    {SDLK_RIGHT, cc::KeyCode::ARROW_RIGHT},
    {SDLK_UP, cc::KeyCode::ARROW_UP},
    {SDLK_DOWN, cc::KeyCode::ARROW_DOWN},
    {SDLK_KP_ENTER, cc::KeyCode::NUMPAD_ENTER},
    {SDLK_KP_PLUS, cc::KeyCode::NUMPAD_PLUS},
    {SDLK_KP_MULTIPLY, cc::KeyCode::NUMPAD_MULTIPLY},
    {SDLK_KP_DIVIDE, cc::KeyCode::NUMPAD_DIVIDE},
    {SDLK_KP_MINUS, cc::KeyCode::NUMPAD_MINUS},
    {SDLK_KP_PERIOD, cc::KeyCode::NUMPAD_DECIMAL},
    {SDLK_KP_BACKSPACE, cc::KeyCode::BACKSPACE},
    {SDLK_NUMLOCKCLEAR, cc::KeyCode::NUM_LOCK},
    {SDLK_HOME, cc::KeyCode::HOME},
    {SDLK_PAGEUP, cc::KeyCode::PAGE_UP},
    {SDLK_PAGEDOWN, cc::KeyCode::PAGE_DOWN},
    {SDLK_END, cc::KeyCode::END},
    {SDLK_COMMA, cc::KeyCode::COMMA},
    {SDLK_PERIOD, cc::KeyCode::PERIOD},
    {SDLK_SLASH, cc::KeyCode::SLASH},
    {SDLK_SPACE, cc::KeyCode::SPACE},
    {SDLK_DELETE, cc::KeyCode::DELETE_KEY},
    {SDLK_CAPSLOCK, cc::KeyCode::CAPS_LOCK},
    {SDLK_KP_0, cc::KeyCode::NUMPAD_0},
    {SDLK_KP_1, cc::KeyCode::NUMPAD_1},
    {SDLK_KP_2, cc::KeyCode::NUMPAD_2},
    {SDLK_KP_3, cc::KeyCode::NUMPAD_3},
    {SDLK_KP_4, cc::KeyCode::NUMPAD_4},
    {SDLK_KP_5, cc::KeyCode::NUMPAD_5},
    {SDLK_KP_6, cc::KeyCode::NUMPAD_6},
    {SDLK_KP_7, cc::KeyCode::NUMPAD_7},
    {SDLK_KP_8, cc::KeyCode::NUMPAD_8},
    {SDLK_KP_9, cc::KeyCode::NUMPAD_9}};

int sdlKeycodeToCocosCode(int sdlCode, int mode) {
    auto it = gKeyMap.find(sdlCode);
    if (it != gKeyMap.end()) {
        return static_cast<int>(it->second);
    }

    if (sdlCode >= SDLK_F1 && sdlCode <= SDLK_F12) {
        // F1 ~ F12
        return 112 + (sdlCode - SDLK_F1);
    } else {
        int code = sdlCode & (~(1 << 30));
        if (code >= SDLK_a && code <= SDLK_z) {
            return 'A' + (code - SDLK_a);
        } else {
            return code;
        }
    }
}

std::unordered_map<cc::ISystemWindow::WindowFlags, SDL_WindowFlags> gWindowFlagMap = {
    {cc::ISystemWindow::CC_WINDOW_FULLSCREEN, SDL_WINDOW_FULLSCREEN},
    {cc::ISystemWindow::CC_WINDOW_OPENGL, SDL_WINDOW_OPENGL},
    {cc::ISystemWindow::CC_WINDOW_SHOWN, SDL_WINDOW_SHOWN},
    {cc::ISystemWindow::CC_WINDOW_HIDDEN, SDL_WINDOW_HIDDEN},
    {cc::ISystemWindow::CC_WINDOW_BORDERLESS, SDL_WINDOW_BORDERLESS},
    {cc::ISystemWindow::CC_WINDOW_RESIZABLE, SDL_WINDOW_RESIZABLE},
    {cc::ISystemWindow::CC_WINDOW_MINIMIZED, SDL_WINDOW_MINIMIZED},
    {cc::ISystemWindow::CC_WINDOW_MAXIMIZED, SDL_WINDOW_MAXIMIZED},
    {cc::ISystemWindow::CC_WINDOW_INPUT_GRABBED, SDL_WINDOW_INPUT_GRABBED},
    {cc::ISystemWindow::CC_WINDOW_INPUT_FOCUS, SDL_WINDOW_INPUT_FOCUS},
    {cc::ISystemWindow::CC_WINDOW_MOUSE_FOCUS, SDL_WINDOW_MOUSE_FOCUS},
    {cc::ISystemWindow::CC_WINDOW_FULLSCREEN_DESKTOP, SDL_WINDOW_FULLSCREEN_DESKTOP},
    {cc::ISystemWindow::CC_WINDOW_FOREIGN, SDL_WINDOW_FOREIGN},
    {cc::ISystemWindow::CC_WINDOW_ALLOW_HIGHDPI, SDL_WINDOW_ALLOW_HIGHDPI},
    {cc::ISystemWindow::CC_WINDOW_MOUSE_CAPTURE, SDL_WINDOW_MOUSE_CAPTURE},
    {cc::ISystemWindow::CC_WINDOW_ALWAYS_ON_TOP, SDL_WINDOW_ALWAYS_ON_TOP},
    {cc::ISystemWindow::CC_WINDOW_SKIP_TASKBAR, SDL_WINDOW_SKIP_TASKBAR},
    {cc::ISystemWindow::CC_WINDOW_UTILITY, SDL_WINDOW_UTILITY},
    {cc::ISystemWindow::CC_WINDOW_TOOLTIP, SDL_WINDOW_TOOLTIP},
    {cc::ISystemWindow::CC_WINDOW_POPUP_MENU, SDL_WINDOW_POPUP_MENU},
    {cc::ISystemWindow::CC_WINDOW_VULKAN, SDL_WINDOW_VULKAN}};

int windowFlagsToSDLWindowFlag(int flags) {
    int sdlFlags = 0;
    for (auto it : gWindowFlagMap) {
        if ((it.first & flags) == it.first) {
            sdlFlags |= it.second;
        }
    }
    return sdlFlags;
}

} // namespace

namespace cc {
WindowsPlatform::WindowsPlatform() = default;
WindowsPlatform::~WindowsPlatform() {
    if (_handle) {
        SDL_DestroyWindow(_handle);
        _handle = nullptr;
    }
    if (_inited) {
        SDL_Quit();
    }
#ifdef USE_WIN32_CONSOLE
    FreeConsole();
#endif
}

int32_t WindowsPlatform::init() {
    UniversalPlatform::init();
#ifdef USE_WIN32_CONSOLE
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

    PVRFrameEnableControlWindow(false);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // Display error message
        CC_LOG_ERROR("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    return 0;
}

int32_t WindowsPlatform::loop() {
    ///////////////////////////////////////////////////////////////////////////
    /////////////// changing timer resolution
    ///////////////////////////////////////////////////////////////////////////
    UINT     TARGET_RESOLUTION = 1; // 1 millisecond target resolution
    TIMECAPS tc;
    UINT     wTimerRes = 0;
    if (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(TIMECAPS))) {
        wTimerRes = std::min(std::max(tc.wPeriodMin, TARGET_RESOLUTION), tc.wPeriodMax);
        timeBeginPeriod(wTimerRes);
    }

    float       dt    = 0.f;
    const DWORD _16ms = 16;

    // Main message loop:
    LARGE_INTEGER nFreq;
    LARGE_INTEGER nLast;
    LARGE_INTEGER nNow;

    LONGLONG actualInterval  = 0LL; // actual frame internal
    LONGLONG desiredInterval = 0LL; // desired frame internal, 1 / fps
    LONG     waitMS          = 0L;

    QueryPerformanceCounter(&nLast);
    QueryPerformanceFrequency(&nFreq);

    onResume();
    while (!_quit) {
        desiredInterval = (LONGLONG)(1.0 / getFps() * nFreq.QuadPart);
        pollEvent();
        QueryPerformanceCounter(&nNow);
        actualInterval = nNow.QuadPart - nLast.QuadPart;
        if (actualInterval >= desiredInterval) {
            nLast.QuadPart = nNow.QuadPart;
            runTask();
            SDL_GL_SwapWindow(_handle);
        } else {
            // The precision of timer on Windows is set to highest (1ms) by 'timeBeginPeriod' from above code,
            // but it's still not precise enough. For example, if the precision of timer is 1ms,
            // Sleep(3) may make a sleep of 2ms or 4ms. Therefore, we subtract 1ms here to make Sleep time shorter.
            // If 'waitMS' is equal or less than 1ms, don't sleep and run into next loop to
            // boost CPU to next frame accurately.
            waitMS = static_cast<LONG>((desiredInterval - actualInterval) * 1000LL / nFreq.QuadPart - 1L);
            if (waitMS > 1L)
                Sleep(waitMS);
        }
    }

    if (wTimerRes != 0)
        timeEndPeriod(wTimerRes);

    onDestory();
    return 0;
}

void WindowsPlatform::handleWindowEvent(SDL_WindowEvent &wevent) {
    WindowEvent ev;
    switch (wevent.event) {
        case SDL_WINDOWEVENT_SHOWN: {
            ev.type = WindowEvent::Type::SHOW;
            dispatchEvent(ev);
            break;
        }
        case SDL_WINDOWEVENT_RESTORED: {
            ev.type = WindowEvent::Type::RESTORED;
            dispatchEvent(ev);
            break;
        }
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
            ev.type   = WindowEvent::Type::SIZE_CHANGED;
            ev.width  = wevent.data1;
            ev.height = wevent.data2;
            dispatchEvent(ev);
            break;
        }
        case SDL_WINDOWEVENT_RESIZED: {
            ev.type   = WindowEvent::Type::RESIZED;
            ev.width  = wevent.data1;
            ev.height = wevent.data2;
            dispatchEvent(ev);
            break;
        }
        case SDL_WINDOWEVENT_HIDDEN: {
            ev.type = WindowEvent::Type::HIDDEN;
            dispatchEvent(ev);
            break;
        }
        case SDL_WINDOWEVENT_MINIMIZED: {
            ev.type = WindowEvent::Type::MINIMIZED;
            dispatchEvent(ev);
            break;
        }
        case SDL_WINDOWEVENT_ENTER: {
            SDL_CaptureMouse(SDL_TRUE);
            break;
        }
        case SDL_WINDOWEVENT_CLOSE: {
            ev.type = WindowEvent::Type::CLOSE;
            dispatchEvent(ev);
            break;
        }
    }
}

void WindowsPlatform::pollEvent() {
    SDL_Event sdlEvent;
    int       cnt = SDL_PollEvent(&sdlEvent);
    if (cnt == 0) {
        return;
    }
    cc::TouchEvent    touch;
    cc::MouseEvent    mouse;
    cc::KeyboardEvent keyboard;
    switch (sdlEvent.type) {
        case SDL_QUIT: {
            _quit = true;
            WindowEvent ev;
            ev.type = WindowEvent::Type::QUIT;
            dispatchEvent(ev);
            break;
        }
        case SDL_WINDOWEVENT: {
            handleWindowEvent(sdlEvent.window);
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            SDL_MouseButtonEvent &event = sdlEvent.button;
            mouse.type                  = MouseEvent::Type::DOWN;
            mouse.x                     = static_cast<float>(event.x);
            mouse.y                     = static_cast<float>(event.y);
            /*if (0 > mouse.x || mouse.x > this->_width || 0 > mouse.y || mouse.y > this->_height) {
                    break;
                }*/
            mouse.button = event.button - 1;
            dispatchEvent(mouse);
            break;
        }
        case SDL_MOUSEBUTTONUP: {
            SDL_MouseButtonEvent &event = sdlEvent.button;
            mouse.type                  = MouseEvent::Type::UP;
            mouse.x                     = static_cast<float>(event.x);
            mouse.y                     = static_cast<float>(event.y);
            mouse.button                = event.button - 1;
            dispatchEvent(mouse);
            break;
        }
        case SDL_MOUSEMOTION: {
            SDL_MouseMotionEvent &event = sdlEvent.motion;
            mouse.type                  = MouseEvent::Type::MOVE;
            mouse.x                     = static_cast<float>(event.x);
            mouse.y                     = static_cast<float>(event.y);
            mouse.button                = 0;
            dispatchEvent(mouse);
            break;
        }
        case SDL_MOUSEWHEEL: {
            SDL_MouseWheelEvent &event = sdlEvent.wheel;
            mouse.type                 = MouseEvent::Type::WHEEL;
            mouse.x                    = static_cast<float>(event.x);
            mouse.y                    = static_cast<float>(event.y);
            mouse.button               = 0; //TODO: direction
            dispatchEvent(mouse);
            break;
        }
        case SDL_FINGERUP: {
            SDL_TouchFingerEvent &event = sdlEvent.tfinger;
            touch.type                  = TouchEvent::Type::ENDED;
            touch.touches               = {TouchInfo(event.x, event.y, (int)event.fingerId)};
            dispatchEvent(touch);
            break;
        }
        case SDL_FINGERDOWN: {
            SDL_TouchFingerEvent &event = sdlEvent.tfinger;
            touch.type                  = TouchEvent::Type::BEGAN;
            touch.touches               = {TouchInfo(event.x, event.y, (int)event.fingerId)};
            dispatchEvent(touch);
            break;
        }
        case SDL_FINGERMOTION: {
            SDL_TouchFingerEvent &event = sdlEvent.tfinger;
            touch.type                  = TouchEvent::Type::MOVED;
            touch.touches               = {TouchInfo(event.x, event.y, (int)event.fingerId)};
            dispatchEvent(touch);
            break;
        }
        case SDL_KEYDOWN: {
            SDL_KeyboardEvent &event = sdlEvent.key;
            auto               mode  = event.keysym.mod;
            keyboard.action          = KeyboardEvent::Action::PRESS;
            keyboard.key             = sdlKeycodeToCocosCode(event.keysym.sym, mode);
            keyboard.altKeyActive    = mode & KMOD_ALT;
            keyboard.ctrlKeyActive   = mode & KMOD_CTRL;
            keyboard.shiftKeyActive  = mode & KMOD_SHIFT;
            //CC_LOG_DEBUG("==> key %d -> code %d", event.keysym.sym, keyboard.key);
            //cc::EventDispatcher::dispatchKeyboardEvent(keyboard);
            dispatchEvent(keyboard);
            break;
        }
        case SDL_KEYUP: {
            SDL_KeyboardEvent &event = sdlEvent.key;
            auto               mode  = event.keysym.mod;
            keyboard.action          = KeyboardEvent::Action::RELEASE;
            keyboard.key             = sdlKeycodeToCocosCode(event.keysym.sym, mode);
            keyboard.altKeyActive    = mode & KMOD_ALT;
            keyboard.ctrlKeyActive   = mode & KMOD_CTRL;
            keyboard.shiftKeyActive  = mode & KMOD_SHIFT;
            dispatchEvent(keyboard);
            //cc::EventDispatcher::dispatchKeyboardEvent(keyboard);
            break;
        }
        default:

            break;
    }
}

bool WindowsPlatform::createWindow(const char *title,
                                   int x, int y, int w,
                                   int h, int flags) {
    if (_inited) {
        return true;
    }
    // Create window
    int sdlFlags = windowFlagsToSDLWindowFlag(flags);
    _handle      = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, sdlFlags);
    if (_handle == nullptr) {
        // Display error message
        CC_LOG_ERROR("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    _inited = true;
    return true;
}

uintptr_t WindowsPlatform::getWindowHandler() const {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(_handle, &wmInfo);
    return reinterpret_cast<uintptr_t>(wmInfo.info.win.window);
}

} // namespace cc
