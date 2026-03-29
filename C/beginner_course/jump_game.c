// =====================================================================
// 공룡 점프 게임 (Google Dino Jump 스타일)
// 스페이스바로 점프, ESC/Q로 종료
// =====================================================================

// =====================================================================
// 필요한 헤더 파일들
// =====================================================================
#include <stdio.h>       // 표준 입출력
#include <stdlib.h>      // 메모리 할당, 난수 생성
#include <conio.h>       // 콘솔 입력 (kbhit, getch)
#include <windows.h>     // Windows API (콘솔 제어, 핸들 등)
#include <time.h>        // 시간/난수 시드
#include <wchar.h>       // 한글 포함 와이드 문자 처리
#include <io.h>          // 파일 디스크립터
#include <fcntl.h>       // 파일 제어 플래그
#include <locale.h>      // 로케일 설정 (한글)
#include <stdarg.h>      // 가변 인자 (wprint 함수용)

// =====================================================================
// 게임 설정 상수
// =====================================================================
#define ESC_KEY 27       // ESC 키 ASCII 코드 (게임 종료용)
#define WIDTH 80         // 화면 가로 크기 (칸)
#define HEIGHT 20        // 화면 세로 크기 (칸)
#define DINO_Y 15        // 공룡 기본 위치 Y좌표 (지면 높이)

// =====================================================================
// 한글 출력 헬퍼 함수
// =====================================================================
// 설명: printf 대신 이 함수를 사용하면 한글(UTF-16)이 깨지지 않고 출력됨
//       WriteConsoleW를 사용하여 Windows 콘솔에 직접 출력
static void wprint(HANDLE hConsole, const wchar_t *fmt, ...) {
    wchar_t buf[2048];              // 출력할 문자열 버퍼
    va_list ap;                     // 가변 인자 리스트
    va_start(ap, fmt);              // 가변 인자 처리 시작
    vswprintf(buf, fmt, ap);        // 형식화된 문자열 생성
    va_end(ap);                     // 가변 인자 처리 종료
    DWORD written = 0;              // 출력된 문자 수 (사용 안 함)
    WriteConsoleW(hConsole, buf, (DWORD)wcslen(buf), &written, NULL);
}

// =====================================================================
// 메인 함수 - 게임의 시작점
// =====================================================================
int main(void) {
    // --- 초기화 단계 ---
    setlocale(LC_ALL, "");          // 한글 출력을 위한 로케일 설정

    // Windows 콘솔 핸들 획득 (콘솔 제어용)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // 콘솔 커서 정보 구조체
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;    // 커서 숨기기 (깜빡임 감소)
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // === 화면 버퍼 준비 ===
    // 게임 화면을 저장할 2차원 배열
    // 각 행의 끝에 널 종료문자(\\0)를 추가하여 문자열로 사용 가능
    static wchar_t screen[HEIGHT][WIDTH + 1];
    int i, j;  // 루프 변수

    // 콘솔 버퍼/윈도우 크기 설정
    // 스크롤이 발생하지 않도록 버퍼 크기를 화면 크기보다 크게 설정
    COORD newSize = {WIDTH, HEIGHT + 6};
    SetConsoleScreenBufferSize(hConsole, newSize);
    SMALL_RECT winRect = {0, 0, WIDTH - 1, HEIGHT + 5};
    SetConsoleWindowInfo(hConsole, TRUE, &winRect);

    // 구분선 버퍼 생성
    // '=' 문자로 WIDTH 크기의 구분선을 만들어 미리 버퍼에 저장
    static wchar_t sep[WIDTH + 1];
    for (i = 0; i < WIDTH; ++i) sep[i] = L'=';
    sep[WIDTH] = L'\\0';  // 문자열 끝 표시

    // === 게임 상태 변수들 ===
    // 공룡 위치 및 상태
    int dino_x = 5;          // 공룡 X 좌표 (화면 좌측, 고정)
    int dino_y = DINO_Y;     // 공룡 Y 좌표 (점프하면 변함)
    int velocity = 0;        // 현재 수직 속도 (음수=위로, 양수=아래로)
    int gravity = 1;         // 중력 가속도 (매 프레임 velocity에 더해짐)
    int is_jumping = 0;      // 점프 상태 플래그 (1=점프 중, 0=지면)

    // 장애물 위치 및 상태
    int obstacle_x = WIDTH - 10;  // 장애물 X 좌표 (우측에서 시작)
    int obstacle_y = DINO_Y;      // 장애물 Y 좌표 (지면 높이)

    // 게임 진행 상태
    int score = 0;           // 피한 장애물 개수 = 점수
    int game_over = 0;       // 게임 종료 플래그 (1이면 루프 탈출)

    // 난수 생성 시드 초기화
    srand((unsigned)time(NULL));

    // === 게임 시작 안내 화면 ===
    wprint(hConsole, L"\\n");
    wprint(hConsole, L"               === 공룡 점프 게임 ===                          \\n");
    wprint(hConsole, L"          스페이스바를 눌러서 점프하세요!                       \\n");
    wprint(hConsole, L"          q 또는 ESC를 눌러 게임을 종료합니다.                 \\n");
    wprint(hConsole, L"          @ 는 공룡, # 는 장애물입니다.                        \\n");
    wprint(hConsole, L"\\n");
    Sleep(1000);  // 1초 동안 안내 화면 표시

    // 화면 갱신용 커서 위치 (항상 (0,0)으로 리셋하여 깜빡임 감소)
    COORD home = {0, 0};

    // =====================================================================
    // 메인 게임 루프 (매 프레임마다 반복)
    // =====================================================================
    while (!game_over) {
        // --- 1단계: 화면 버퍼 초기화 ---
        // 매 프레임마다 화면을 빈 상태로 만듦 (공백 문자로 채우기)
        for (i = 0; i < HEIGHT; i++) {
            for (j = 0; j < WIDTH; j++) 
                screen[i][j] = L' ';    // 공백으로 채움
            screen[i][WIDTH] = L'\\0';  // 각 행 끝에 널 종료문자 추가
        }

        // --- 2단계: 지면 그리기 ---
        // Y좌표 DINO_Y+3 위치에 '-' 문자로 지면을 그림
        // 공룡은 DINO_Y 높이에 있으므로 지면은 3칸 아래 (공룡의 가시적 크기 고려)
        for (j = 0; j < WIDTH; j++) 
            screen[DINO_Y + 3][j] = L'-';

        // --- 3단계: 키 입력 처리 ---
        // kbhit(): 키가 입력되었는지 논블로킹 방식으로 확인
        // getch(): 입력된 키의 ASCII 코드 반환
        if (kbhit()) {
            int key = getch();
            
            // 스페이스바: 점프 시작 (이미 점프 중이면 재점프 불가)
            if (key == ' ' && !is_jumping) {
                is_jumping = 1;         // 점프 상태 활성화
                velocity = -12;         // 초기 수직 속도 설정 (위쪽 방향)
            }
            
            // q 또는 ESC: 게임 종료
            if (key == 'q' || key == 'Q' || key == ESC_KEY) {
                game_over = 1;          // 게임 루프 종료 플래그 설정
                break;
            }
        }

        // --- 4단계: 점프 물리 계산 (중력 시뮬레이션) ---
        // 점프 중일 때만 이 로직 실행
        if (is_jumping) {
            velocity += gravity;        // 중력으로 인해 속도 감소 (아래로 가속)
            dino_y += velocity;         // 속도만큼 Y 좌표 이동
            
            // 지면에 닿으면 점프 종료
            if (dino_y >= DINO_Y) {
                dino_y = DINO_Y;        // Y 좌표를 정확히 지면 높이로 설정
                is_jumping = 0;         // 점프 상태 해제
                velocity = 0;           // 속도 초기화
            }
            
            // 화면 위쪽으로 나가는 것 방지
            if (dino_y < 0) {
                dino_y = 0;
            }
        }

        // --- 5단계: 공룡 그리기 ---
        // 공룡을 '@' 문자로 화면에 표시
        // 화면 범위 내에 있을 때만 그림
        if (dino_y >= 0 && dino_y < HEIGHT) 
            screen[dino_y][dino_x] = L'@';

        // --- 6단계: 장애물 처리 ---
        // 장애물을 매 프레임마다 왼쪽으로 이동 (우측좌측 이동 효과)
        obstacle_x--;
        
        // 장애물이 화면 범위 내에 있으면 그리기
        if (obstacle_x >= 0 && obstacle_x < WIDTH) {
            screen[obstacle_y][obstacle_x] = L'#';  // '#' 문자로 장애물 표시
            
            // 충돌 판정: 공룡과 장애물의 좌표가 같으면 충돌!
            if (dino_x == obstacle_x && dino_y == obstacle_y) {
                game_over = 1;          // 게임 즉시 종료
                break;
            }
        }

        // 장애물이 화면 왼쪽 끝을 완전히 벗어나면 새 장애물 생성
        if (obstacle_x < -5) {
            score++;                    // 점수 증가 (성공적으로 회피함)
            obstacle_x = WIDTH - 5;     // 새 장애물을 화면 우측에 배치
            obstacle_y = DINO_Y;        // 장애물 높이는 지면 고정
        }

        // --- 7단계: 화면 출력 (깜빡임 최소화 방식) ---
        // system("cls")를 사용하면 화면이 완전히 지워져 깜빡임 발생
        // 대신: 커서를 (0,0)으로 이동  기존 위치에 덮어쓰기  깜빡임 최소화
        SetConsoleCursorPosition(hConsole, home);

        // 게임 상태 정보 출력 (상단 정보)
        wprint(hConsole, L"점수: %d | 상태: %ls | 장애물 위치: %d\\n", 
               score, 
               is_jumping ? L"점프 중" : L"대기 중", 
               obstacle_x);
        
        // 상단 구분선 출력
        wprint(hConsole, L"%ls\\n", sep);

        // 게임 화면 전체 그리기 (모든 행 한 번에 출력)
        for (i = 0; i < HEIGHT; i++) {
            wprint(hConsole, L"%ls\\n", screen[i]);
        }

        // 하단 구분선
        wprint(hConsole, L"%ls\\n", sep);
        
        // 조작 안내
        wprint(hConsole, L"조작: 스페이스바(점프) | q 또는 ESC(종료)\\n");

        // 프레임 지연 (게임 속도 조절 & 깜빡임 감소)
        // 60ms  16-17 FPS (느린 속도이지만 가독성 우선)
        Sleep(60);
    }

    // =====================================================================
    // 게임 오버 화면 표시
    // =====================================================================
    // 커서를 맨 위로 이동하여 게임 오버 메시지 출력
    SetConsoleCursorPosition(hConsole, home);
    wprint(hConsole, L"\\n");
    wprint(hConsole, L"\\n");
    wprint(hConsole, L"                        게임 오버!                              \\n");
    wprint(hConsole, L"                      최종 점수: %d점                           \\n", score);
    wprint(hConsole, L"\\n");
    wprint(hConsole, L"\\n");

    // 게임 종료 후 커서 다시 표시 (정상 상태로 복원)
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // 프로그램 종료 전 사용자 입력 대기
    wprint(hConsole, L"아무키나 누르면 종료합니다...\\n");
    getch();

    return 0;
}
