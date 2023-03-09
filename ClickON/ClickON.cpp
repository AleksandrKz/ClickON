// ClickON.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "ClickON.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HWND hWnd = NULL;
HHOOK hHook = NULL;
const int MAX_LEN = 100;
HWND edit_box = NULL;
HWND text_X = NULL;
HWND text_Y = NULL;
HWND updown = NULL;
const int UD_MAX_POS = 999;
const int UD_MIN_POS = 1;
const int width_win = 200;
const int height_win = 255;
const long style_win = (WS_SYSMENU);
HINSTANCE hInst = NULL;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

//ClickON параметры
POINT CursorXY{0,0};
UINT_PTR timer_period_id = 1;
UINT_PTR timer_pausa_id = 2;
BOOL timer_is_set = 0;
BOOL push_btn = 0;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK keyboard_hook(const int, const WPARAM, const LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_hook, NULL, 0);
    if (hHook == NULL)
    {
        exit(1);
    }

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLICKON, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLICKON));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLICKON);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   hWnd = CreateWindowW(szWindowClass, szTitle, style_win,
      CW_USEDEFAULT, 0, width_win, height_win, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UINT code;
    LPNMUPDOWN lpnmud;
    TCHAR szText[MAX_LEN];
    switch (message)
    {
    case WM_CREATE:
        {
            CreateWindow(_T("BUTTON"), _T("Координаты курсора"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                5, 5, 175, 45, hWnd, 0, GetModuleHandle(0), 0);
            text_X = CreateWindow(_T("STATIC"), _T("x: 0"), WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                15, 25, 80, 20, hWnd, 0, GetModuleHandle(0), 0);
            text_Y = CreateWindow(_T("STATIC"), _T("y: 0"), WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                105, 25, 60, 20, hWnd, 0, GetModuleHandle(0), 0);
            CreateWindow(_T("BUTTON"), _T("Период кликов(1-999с.):"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                5, 55, 175, 55, hWnd, 0, GetModuleHandle(0), 0);
            edit_box = CreateWindow(_T("EDIT"), _T("5"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER | ES_READONLY | EN_KILLFOCUS,
                15, 80, 100, 20, hWnd, 0, GetModuleHandle(0), 0);
            updown = CreateWindow(_T("msctls_updown32"), NULL, WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS,
                115, 80, 100, 20, hWnd, 0, GetModuleHandle(0), 0);
            CreateWindow(_T("BUTTON"), _T("Управление:"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                5, 115, 175, 60, hWnd, 0, GetModuleHandle(0), 0);
            CreateWindow(_T("STATIC"), _T("Ctrl - получить коорд."), WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                15, 130, 150, 20, hWnd, 0, GetModuleHandle(0), 0);
            CreateWindow(_T("STATIC"), _T("Shift - старт/стоп"), WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                15, 148, 150, 20, hWnd, 0, GetModuleHandle(0), 0);
            CreateWindow(_T("STATIC"), _T("Aleksandr Kuzyk"), WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
                30, 180, 130, 20, hWnd, 0, GetModuleHandle(0), 0);
            CreateWindow(_T("STATIC"), _T("© 2021"), WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
                60, 195, 57, 20, hWnd, 0, GetModuleHandle(0), 0);
            SendMessageW(updown, UDM_SETBUDDY, (WPARAM)edit_box, 0);
            SendMessageW(updown, UDM_SETRANGE, 0, MAKELPARAM(UD_MAX_POS, UD_MIN_POS));
            SendMessageW(updown, UDM_SETPOS32, 0, 0);
        }
        break;
    case WM_TIMER:
        {
        if (wParam == timer_period_id)
            {
                SetCursorPos(CursorXY.x, CursorXY.y);
                SetTimer(hWnd, timer_pausa_id, 200, NULL);
            }
        if(wParam == timer_pausa_id)
            if (!push_btn)
            {
                mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, CursorXY.x, CursorXY.y, 0, 0);
                push_btn = 1;
            }
            else
            {
                mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, CursorXY.x, CursorXY.y, 0, 0);
                push_btn = 0;
                KillTimer(hWnd, timer_pausa_id);
            }
        }
        break;
    case WM_LBUTTONUP:
        {
            //SetFocus(hWnd);
        }
        break;
    case WM_KEYDOWN:
        {
        if (wParam == VK_LCONTROL || wParam == VK_RCONTROL)
            {
                GetCursorPos(&CursorXY);
                wsprintf(szText, _T("x: %i"), CursorXY.x);
                SetWindowText(text_X, szText);
                wsprintf(szText, _T("y: %i"), CursorXY.y);
                SetWindowText(text_Y, szText);
                //TCHAR szText[50];
                //wsprintf(szText, _T("Position:\ncursor X: %i\ncursor Y: %i"), CursorXY.x, CursorXY.y);
                //MessageBoxW(hWnd, szText, _T("Cursor position"), MB_ICONINFORMATION);
            }
        if(wParam == VK_LSHIFT || wParam == VK_RSHIFT)
            {
                //pausa-start
                if (!timer_is_set)
                {
                    UINT32 sec = 0;
                    GetWindowText(edit_box, szText, MAX_LEN - 1);
                    sec = _tstoi((const wchar_t *)szText);
                    if (sec < 1) sec = 1;
                    if (sec > 999) sec = 999;
                    SetTimer(hWnd, timer_period_id, sec*1000, NULL);
                    timer_is_set = 1;
                }
                else
                {
                    KillTimer(hWnd, timer_period_id);
                    timer_is_set = 0;
                }
            }
        if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
        }
        break;
    case WM_NOTIFY:
        {
            code = ((LPNMHDR)lParam)->code;
            if (code == UDN_DELTAPOS)
            {
                lpnmud = (NMUPDOWN*)lParam;
                int value = lpnmud->iPos + lpnmud->iDelta;
                if (value < UD_MIN_POS) value = UD_MIN_POS;
                if (value > UD_MAX_POS) value = UD_MAX_POS;
                wsprintf(szText, _T("%i"), value);
                SetWindowText(edit_box, szText);
                if(timer_is_set)
                    SetTimer(hWnd, timer_period_id, value * 1000, NULL);
                SetFocus(hWnd);
            }
        }
        break;
    case WM_COMMAND:
        {
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK keyboard_hook(const int code, const WPARAM wParam, const LPARAM lParam)
{
    if (wParam == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD wVirtKey = kbdStruct->vkCode;
        //DWORD wScanCode = kbdStruct->scanCode;
        SendMessageW(hWnd, WM_KEYDOWN, wVirtKey, NULL);
    }
    return CallNextHookEx(hHook, code, wParam, lParam);
}