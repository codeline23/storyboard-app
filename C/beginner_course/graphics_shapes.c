// =====================================================================
// Windows 그래픽 모드 - 도형 그리기 프로그램 (한글 지원 버전)
// =====================================================================
// 직선, 사각형, 원, 삼각형을 그리는 프로그램
// 한글 텍스트가 올바르게 표시됨
// =====================================================================

#include <windows.h>
#include <stdio.h>
#include <math.h>

// =====================================================================
// 윈도우 설정
// =====================================================================
#define WINDOW_WIDTH 800        // 윈도우 가로 크기
#define WINDOW_HEIGHT 600       // 윈도우 세로 크기
#define PI 3.14159265359        // 원주율

// 전역 변수
HWND hwnd;                      // 윈도우 핸들
HDC hdc;                        // 디바이스 컨텍스트 (그리기 도구)

// =====================================================================
// 도형 그리기 함수들
// =====================================================================

// --- 직선 그리기 ---
// 두 점 (x1, y1)에서 (x2, y2)로 직선을 그림
// color: RGB 색상 (0xBBGGRR 형식)
void drawLine(int x1, int y1, int x2, int y2, COLORREF color) {
    HPEN pen = CreatePen(PS_SOLID, 2, color);  // 펜 생성 (굵기 2)
    SelectObject(hdc, pen);                     // 현재 펜으로 설정
    MoveToEx(hdc, x1, y1, NULL);               // 시작점 이동
    LineTo(hdc, x2, y2);                       // 끝점까지 직선 그리기
    DeleteObject(pen);                         // 펜 해제
}

// --- 사각형 그리기 ---
// (x1, y1)에서 (x2, y2)까지의 사각형을 그림
// color: 테두리 색상
// filled: 1이면 채워진 사각형, 0이면 테두리만
void drawRectangle(int x1, int y1, int x2, int y2, COLORREF color, int filled) {
    HPEN pen = CreatePen(PS_SOLID, 2, color);
    SelectObject(hdc, pen);
    
    if (filled) {
        // 채워진 사각형
        HBRUSH brush = CreateSolidBrush(color);
        SelectObject(hdc, brush);
        Rectangle(hdc, x1, y1, x2, y2);
        DeleteObject(brush);
    } else {
        // 테두리만
        Rectangle(hdc, x1, y1, x2, y2);
    }
    DeleteObject(pen);
}

// --- 원 그리기 ---
// 중심 (cx, cy)에서 반지름 radius인 원을 그림
// filled: 1이면 채워진 원, 0이면 테두리만
void drawCircle(int cx, int cy, int radius, COLORREF color, int filled) {
    HPEN pen = CreatePen(PS_SOLID, 2, color);
    SelectObject(hdc, pen);
    
    if (filled) {
        HBRUSH brush = CreateSolidBrush(color);
        SelectObject(hdc, brush);
        Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
        DeleteObject(brush);
    } else {
        Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
    }
    DeleteObject(pen);
}

// --- 삼각형 그리기 ---
// 세 꼭짓점 (x1,y1), (x2,y2), (x3,y3)으로 삼각형 그리기
// filled: 1이면 채워진 삼각형, 0이면 테두리만
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color, int filled) {
    // 삼각형의 세 꼭짓점을 배열에 저장
    POINT points[3] = {{x1, y1}, {x2, y2}, {x3, y3}};
    
    HPEN pen = CreatePen(PS_SOLID, 2, color);
    SelectObject(hdc, pen);
    
    if (filled) {
        // 채워진 삼각형
        HBRUSH brush = CreateSolidBrush(color);
        SelectObject(hdc, brush);
        Polygon(hdc, points, 3);  // 3개의 점으로 다각형 그리기
        DeleteObject(brush);
    } else {
        // 테두리만
        Polygon(hdc, points, 3);
    }
    DeleteObject(pen);
}

// =====================================================================
// 한글 텍스트 그리기 헬퍼 함수
// =====================================================================
void drawKoreanText(int x, int y, const wchar_t *text, int fontSize) {
    HFONT hFont = CreateFontW(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                              FF_DONTCARE, L"맑은고딕");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
    
    TextOutW(hdc, x, y, text, wcslen(text));
    
    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}

// =====================================================================
// 윈도우 콜백 함수 (윈도우 이벤트 처리)
// =====================================================================
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            // 화면을 다시 그려야 할 때 호출됨
            PAINTSTRUCT ps;
            hdc = BeginPaint(hwnd, &ps);
            
            // 배경을 흰색으로 채우기
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &rect, whiteBrush);
            DeleteObject(whiteBrush);
            
            // =========================================================
            // 도형 그리기 시작
            // =========================================================
            
            // 1. 빨간 직선 (좌측 상단에서 우측 중간으로)
            drawLine(50, 50, 300, 150, RGB(255, 0, 0));
            
            // 2. 파란 사각형 (테두리만, 좌측)
            drawRectangle(50, 200, 200, 350, RGB(0, 0, 255), 0);
            
            // 3. 초록색 채워진 사각형 (우측)
            drawRectangle(350, 200, 550, 350, RGB(0, 255, 0), 1);
            
            // 4. 빨간 원 (테두리만, 상단 중앙)
            drawCircle(400, 100, 50, RGB(255, 0, 0), 0);
            
            // 5. 노란색 채워진 원 (하단 중앙)
            drawCircle(400, 480, 50, RGB(255, 255, 0), 1);
            
            // 6. 검은색 삼각형 (테두리만, 좌측 하단)
            drawTriangle(100, 450, 50, 550, 150, 550, RGB(0, 0, 0), 0);
            
            // 7. 분홍색 채워진 삼각형 (우측 하단)
            drawTriangle(550, 450, 500, 550, 600, 550, RGB(255, 0, 255), 1);
            
            // 8. 청록색 직선 (대각선)
            drawLine(600, 50, 750, 500, RGB(0, 255, 255));
            
            // =========================================================
            // 한글 텍스트 출력 (도형 설명)
            // =========================================================
            SetBkMode(hdc, TRANSPARENT);  // 텍스트 배경 투명
            SetTextColor(hdc, RGB(0, 0, 0));  // 텍스트 색상: 검은색
            
            // 각 도형 위에 한글 라벨 표시
            drawKoreanText(100, 160, L"직선", 14);
            drawKoreanText(50, 370, L"사각형", 14);
            drawKoreanText(380, 370, L"채우기", 14);
            drawKoreanText(350, 45, L"원", 14);
            drawKoreanText(350, 550, L"원", 14);
            drawKoreanText(60, 570, L"삼각형", 14);
            drawKoreanText(520, 570, L"삼각", 14);
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_CLOSE:
            // 닫기 버튼 클릭 시
            DestroyWindow(hwnd);
            break;
            
        case WM_DESTROY:
            // 윈도우 제거 시
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// =====================================================================
// 메인 함수 - 프로그램 시작점
// =====================================================================
int main(void) {
    // 윈도우 클래스 등록
    const char* CLASS_NAME = "GraphicsWindow";
    
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;           // 윈도우 콜백 함수
    wc.lpszClassName = CLASS_NAME;          // 클래스 이름
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);  // 커서 (화살표)
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);  // 아이콘
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);  // 배경색
    
    if (!RegisterClassA(&wc)) {
        printf("Window registration failed\n");
        return -1;
    }
    
    // 윈도우 생성 (유니코드 제목 사용)
    hwnd = CreateWindowExW(
        0,                      // 확장 스타일
        L"GraphicsWindow",       // 클래스 이름 (유니코드)
        L"기하도형 그리기 - Korean Graphics Demo",  // 윈도우 제목 (한글)
        WS_OVERLAPPEDWINDOW,    // 윈도우 스타일
        100, 100,               // 윈도우 위치 (x, y)
        WINDOW_WIDTH, WINDOW_HEIGHT,  // 윈도우 크기
        NULL, NULL, NULL, NULL
    );
    
    if (!hwnd) {
        printf("Window creation failed\n");
        return -1;
    }
    
    // 윈도우 표시
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    // 메시지 루프 (윈도우 메시지 처리)
    MSG msg = {0};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);  // 메시지 변환
        DispatchMessageA(&msg);  // 메시지 처리
    }
    
    printf("Program terminated\n");
    return 0;
}
