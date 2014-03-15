#include<Windows.h>
#include<time.h>
#include<memory>
#include<string>
#include<assert.h>
#include"Renderer.h"

const char *MAIN_WIN_CLASS_NAME = "Test Project";
LONG UPDATES_PER_SECOND = 60;

DOUBLE SECONDS_PER_UPDATE = 1.0 / (DOUBLE)UPDATES_PER_SECOND;
DWORD MILLISECONDS_PER_UPDATE = (DWORD)(SECONDS_PER_UPDATE * 1000.0);

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

const UINT NUM_KEYS = 256;

D3DBase *g_demo;
BOOL g_keyArray[NUM_KEYS];

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) 
{
	UNREFERENCED_PARAMETER( prevInstance );
	UNREFERENCED_PARAMETER( cmdLine );

	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = MAIN_WIN_CLASS_NAME;

	if (!RegisterClassEx(&wndClass)) return -1;

   int width = 1024;
   int height = 768;

	RECT rc = { 0, 0, width, height};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindowA(MAIN_WIN_CLASS_NAME, "Blank Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hwnd) return -1;
	
	ShowWindow(hwnd, cmdShow);

   for( UINT i = 0; i < NUM_KEYS; i++) g_keyArray[i] = false;

	// Initialize direct3d stuff 
	std::auto_ptr<Renderer> demo(new Renderer());
   g_demo = demo.get();
	bool result = demo->Initialize(hInstance, hwnd);
   if (!result) return -1;

	MSG msg = { 0 };

   const double MILLISECONDS_PER_SECOND = 1000.0f;
   __int64 countsPerSec;
   QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
   double secondsPerCount = 1.0 / countsPerSec;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{
         __int64 A = 0;
         QueryPerformanceCounter((LARGE_INTEGER*)&A);

         demo->Update(0.0f, g_keyArray);
			demo->Render();

         __int64 B = 0;
         QueryPerformanceCounter((LARGE_INTEGER*)&B);
         double dt = MILLISECONDS_PER_SECOND * secondsPerCount * (B - A);
         DWORD waitTime = MILLISECONDS_PER_UPDATE - (DWORD)dt;

         if (waitTime > 0) Sleep(MILLISECONDS_PER_UPDATE); 
		}
	}

	demo->Shutdown();

	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT paintStruct;
	HDC hDC;

	switch (message)
	{
	case WM_KEYDOWN:
      g_keyArray[wParam] = true;
      break;
	case WM_KEYUP:
      g_keyArray[wParam] = false;
      break;
	case WM_PAINT:
		hDC = BeginPaint(hwnd, &paintStruct);
		EndPaint(hwnd, &paintStruct);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}