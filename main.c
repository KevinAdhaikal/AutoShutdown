#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

char usingWhile = 1;
char usingKeyboard = 0;
char UsingTimerShutDown = 0;
char isPressedNo = 0;
char numberTemp[10];
char currentDir[PATH_MAX];

int currentWaktu = 0;
int waktuLimit = 0;
int currentCountDown = 0, currentCountDown2 = 0;
int countDown = 0;

long lastX = 0;
long lastY = 0;
long FilterMousePosX = 0;
long FilterMousePosY = 0;

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
TCHAR szClassName[ ] = _T("AutoShutdown");
HWND tField, buttonYes, buttonNo, mainMessage, mainWindow;
MSG messages, keyMessage;
HHOOK hHook;
DWORD IDThread[3];
POINT mousePoint;
HANDLE HThread[3];

char* myitoa(int num) {
    if (numberTemp == NULL) return NULL;
    sprintf(numberTemp, "%d", num);
    return numberTemp;
}

int getCharTotal(char* varStr, char valChar) {
    int res = 0;
    for (int a = 0; a < strlen(varStr); a++) if (varStr[a] == valChar) res++;
    return res;
}

LRESULT CALLBACK keyboard_hook(const int code, const WPARAM wParam, const LPARAM lParam) {
	if (wParam == WM_KEYDOWN) usingKeyboard = 1;
	return CallNextHookEx(hHook, code, wParam, lParam);
}

void loadConfig() {
    char resultWindowConf[33];
    memset(resultWindowConf, 0, 33);
    snprintf(resultWindowConf, 33, "%c:/AutoShutdown/AutoShutdown.conf", currentDir[0]);
    FILE* fp = fopen(resultWindowConf, "r");
    if (!fp) {
        waktuLimit = 5;
        countDown = 30;
        FilterMousePosX = 0;
        FilterMousePosY = 0;
    }
    else {
        fseek(fp, 0, SEEK_END);
        int fSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char* fData = malloc(fSize);
        fread(fData, fSize, 1, fp);
        fclose(fp);
        memcpy(&waktuLimit, fData, 4);
        memcpy(&countDown, fData + 4, 4);
        memcpy(&FilterMousePosX, fData + 8, 4);
        memcpy(&FilterMousePosY, fData + 12, 4);
        free(fData);
    }
}

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    getcwd(currentDir, PATH_MAX);
    if (lpszArgument[0] == '1') {
        HWND hWnd = GetConsoleWindow();
        ShowWindow( hWnd, SW_HIDE );

        WNDCLASSEX wincl;        /* Data structure for the windowclass */

        wincl.hInstance = hThisInstance;
        wincl.lpszClassName = szClassName;
        wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
        wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
        wincl.cbSize = sizeof (WNDCLASSEX);

        wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
        wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
        wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
        wincl.lpszMenuName = NULL;                 /* No menu */
        wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
        wincl.cbWndExtra = 0;                      /* structure or the window instance */
        wincl.hbrBackground = (HBRUSH)COLOR_WINDOW;
        if (!RegisterClassEx (&wincl)) return 0;
        RECT rect;

        GetClientRect(GetDesktopWindow(), &rect);
        rect.left = (rect.right/2) - (330/2);
        rect.top = (rect.bottom/2) - (120/2);

        mainWindow = CreateWindowEx (0,                   /* Extended possibilites for variation */
                                     szClassName,         /* Classname */
                                     _T("AutoShutdown"),       /* Title Text */
                                     WS_BORDER, /* default window */
                                     rect.left,       /* Windows decides the position */
                                     rect.top,       /* where the window ends up on the screen */
                                     330,                 /* The programs width */
                                     120,                 /* and height in pixels */
                                     HWND_DESKTOP,        /* The window is a child-window to desktop */
                                     NULL,                /* No menu */
                                     hThisInstance,       /* Program Instance handler */
                                     NULL                 /* No Window Creation data */
                                     );

        ShowWindow (mainWindow, SW_HIDE);
        SetWindowPos(mainWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        while (GetMessage (&messages, NULL, 0, 0)) {
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }
        return messages.wParam;
    } else {
        printf("[+] AutoShutdown (C) Kevin Adhaikal\n");
        printf("\n[+] Jika anda tidak mau menggunakan AutoShutdown, anda bisa mengclose console ini");
        printf("\n[+] Jika konsol nya hilang sendiri, berarti aplikasi nya berjalan tanpa ada masalah!\n");
        printf("\n[+] AutoShutdown dimulai dalam 5");
        Sleep(1000);
        printf("\b4");
        Sleep(1000);
        printf("\b3");
        Sleep(1000);
        printf("\b2");
        Sleep(1000);
        printf("\b1");
        Sleep(1000);
        char* currentDirArr[getCharTotal(currentDir, '\\') + 1];
        char* strtokVar = strtok(currentDir, "\\");
        int currentDirPos = 0;
        while (strtokVar != NULL) {
            currentDirArr[currentDirPos++] = strtokVar;
            strtokVar = strtok(NULL, "\\");
        }
        char resultWindowExe[40];
        memset(resultWindowExe, 0, 40);
        snprintf(resultWindowExe, 40, "start %s/AutoShutdown/AutoShutdown.exe 1", currentDirArr[0]);
        system(resultWindowExe);
        exit(0);
    }
    return 0;
}

void keyEvent() {
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_hook, NULL, 0);
    while (GetMessage(&keyMessage, NULL, 0, 0));
}

void changeButtonYes() {
    isPressedNo = 0;
    for (currentCountDown = countDown; currentCountDown > 0; currentCountDown--) {
        char strTime[5 + strlen(myitoa(currentCountDown)) + 1];
        sprintf(strTime, "Yes (%d)", currentCountDown);
        SetWindowText(buttonYes, strTime);
        Sleep(1000);
        currentCountDown2++;
    }
    if ((UsingTimerShutDown || currentCountDown2 == countDown) && !isPressedNo) {
        currentCountDown2 = 0;
        ShowWindow(mainWindow, SW_HIDE);
        system("shutdown -s -f -t 0");
    } else currentCountDown2 = 0;
}

void runningEvent() {
    HThread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)keyEvent, NULL, 0, &IDThread[0]);

    while (usingWhile) {
        Sleep(1000);
        GetCursorPos(&mousePoint);

        if ((mousePoint.x < lastX + FilterMousePosX + 1 && mousePoint.x > lastX - (FilterMousePosX + 1)) && (mousePoint.y < lastY + FilterMousePosY + 1 && mousePoint.y > lastY - (FilterMousePosY + 1))) {
            currentWaktu++;
            if (currentWaktu > waktuLimit) usingWhile = 0;
        } else {
            currentWaktu = 0;
            usingKeyboard = 0;
        }
        lastX = mousePoint.x;
        lastY = mousePoint.y;
    }

    usingWhile = 1;
    currentWaktu = 0;

    PostThreadMessage(IDThread[0], WM_QUIT, 0, 0);
    CloseHandle(HThread[0]);
    ShowWindow(mainWindow, SW_SHOW);

    HThread[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)changeButtonYes, NULL, 0, &IDThread[1]);
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            tField = CreateWindow("STATIC",
                                  "Apakah komputer ini mau di matikan sekarang",
                                  WS_VISIBLE | WS_CHILD,
                                  15,5,320,20,
                                  hwnd, NULL, NULL, NULL);

            buttonYes = CreateWindow("BUTTON",
                                     "Yes",
                                     WS_VISIBLE | WS_CHILD | WS_BORDER,
                                     200, 40, 100, 40,
                                     hwnd, (HMENU)1, NULL, NULL);

            buttonNo = CreateWindow("BUTTON",
                                    "No",
                                    WS_VISIBLE | WS_CHILD | WS_BORDER,
                                    20, 40, 100, 40,
                                    hwnd, (HMENU)2, NULL, NULL);
            loadConfig();
            HThread[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)runningEvent, NULL, 0, &IDThread[2]);
            break;
        }
        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case 1: {
                    UsingTimerShutDown = 1;
                    currentCountDown = 0;
                    ShowWindow(mainWindow, SW_HIDE);
                    break;
                }
                case 2: {
                    UsingTimerShutDown = 0;
                    currentCountDown = 0;
                    isPressedNo = 1;
                    CloseHandle(HThread[1]);
                    CloseHandle(HThread[2]);
                    ShowWindow(mainWindow, SW_HIDE);
                    HThread[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)runningEvent, NULL, 0, &IDThread[2]);
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        case WM_CLOSE: {
            break;
        }
        default: {
            return DefWindowProc (hwnd, message, wParam, lParam);
        }
    }
    return 0;
}
