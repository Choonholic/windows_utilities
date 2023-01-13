// WinKey.cpp

#include "stdafx.h"
#include "WinKey.h"
#include <shellapi.h>

#define MAX_LOADSTRING		100
#define	WM_USER_SHELLICON	(WM_USER + 0x0116)

HINSTANCE		hInst;
TCHAR			szTitle[MAX_LOADSTRING];
TCHAR			szWindowClass[MAX_LOADSTRING];
NOTIFYICONDATA	notifyIconData;
BOOL			bWinKeyEnable = TRUE;
HHOOK			hHook;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL				AddTrayIcon(HINSTANCE hInstance, HWND hWnd);
BOOL				ChangeTrayIcon(HINSTANCE hInstance, HWND hWnd);
BOOL				RemoveTrayIcon(void);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG		msg;
	HACCEL	hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINKEY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable	= LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINKEY));
	hHook		= SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnhookWindowsHookEx(hHook);
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINKEY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINKEY);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WINKEY));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND	hWnd;

	hInst	= hInstance;
	hWnd	= CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	AddTrayIcon(hInst, hWnd);
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int		wmId;
	int		wmEvent;
	POINT	pt;

	switch (message)
	{
	case WM_USER_SHELLICON:
		{
			switch (LOWORD(lParam))
			{
			case WM_LBUTTONDOWN:
				{
					bWinKeyEnable = !bWinKeyEnable;

					ChangeTrayIcon(hInst, hWnd);
				}
				break;
			case WM_RBUTTONDOWN:
				{
					UINT	uFlag	= MF_BYPOSITION | MF_STRING;
					HMENU	hMenu	= CreatePopupMenu();

					GetCursorPos(&pt);
					InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_ENABLE, (bWinKeyEnable ? L"Disable" : L"Enable"));
					InsertMenu(hMenu, 0xFFFFFFFF, MF_SEPARATOR, 0, NULL);
					InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit");
					SetForegroundWindow(hWnd);
					TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
				}
				break;
			}
		}
		break;
	case WM_COMMAND:
		{
			wmId	= LOWORD(wParam);
			wmEvent	= HIWORD(wParam);

			switch (wmId)
			{
			case IDM_ENABLE:
				{
					bWinKeyEnable	= !bWinKeyEnable;

					ChangeTrayIcon(hInst, hWnd);
				}
				break;
			case IDM_EXIT:
				{
					RemoveTrayIcon();
					DestroyWindow(hWnd);
				}
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

BOOL AddTrayIcon(HINSTANCE hInstance, HWND hWnd)
{
	notifyIconData.cbSize			= sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd				= (HWND)hWnd;
	notifyIconData.uID				= IDI_WINKEY;
	notifyIconData.uFlags			= NIF_ICON | NIF_MESSAGE;
	notifyIconData.hIcon			= LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(bWinKeyEnable ? IDI_WINKEY : IDI_WINKEY_DISABLED));
	notifyIconData.uCallbackMessage	= WM_USER_SHELLICON;

	return Shell_NotifyIcon(NIM_ADD, &notifyIconData);
}

BOOL ChangeTrayIcon(HINSTANCE hInstance, HWND hWnd)
{
	notifyIconData.uFlags = NIF_ICON;
	notifyIconData.hIcon = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(bWinKeyEnable ? IDI_WINKEY : IDI_WINKEY_DISABLED));

	return Shell_NotifyIcon(NIM_MODIFY, &notifyIconData);
}

BOOL RemoveTrayIcon(void)
{
	return Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	BOOL	bHooked	= FALSE;

	if (bWinKeyEnable == FALSE)
	{
		if (nCode == HC_ACTION)
		{
			switch (wParam)
			{
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
				{
					PKBDLLHOOKSTRUCT	p	= (PKBDLLHOOKSTRUCT)lParam;

					if (bHooked = ((p->vkCode & VK_LWIN) == VK_LWIN))
					{
						break;
					}
				}
				break;
			}
		}
	}

	return (bHooked ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}
