# 🖥️ UI 시스템

> Harmonia의 모든 사용자 인터페이스

---

## 📌 개요

Harmonia의 UI는 **C++ UMG**와 **Blueprint UMG**를 혼합하여 구현했습니다.  
실시간 그리기가 필요한 부분은 C++로, 일반 UI는 Blueprint로 제작했습니다.

---

## 🎮 UI 구성 요소

---

### 🔷 게임 HUD

![HUD 시스템](../Image/System/UI/hud_main.png)

**플레이 중 항상 표시되는 UI**

게임 플레이에 필요한 최소한의 정보만 표시합니다.

**포함 요소:**
- 크로스헤어 (조준점)
- 상호작용 프롬프트 (E키, R키 등)
- 퀘스트 위젯 (각 레벨 목표)
- 리셋 가이드 (Z키 안내)

**기술 스택:** `C++ UMG` `Blueprint UMG`

**📖 상세 보기:** [HUD 시스템 →](./System/UI/HUD_시스템.md)

---

### 🔷 인벤토리 & 노트

![인벤토리](../Image/System/UI/inventory_main.png)

**아이템 관리 및 정보 확인**

Tab 키로 열 수 있는 인벤토리와 노트 시스템입니다.

**조작 방법:**
- **Tab:** 인벤토리 열기/닫기
- **Q/E:** 슬롯 이동
- **우클릭:** 노트 열기

**주요 아이템:**
- **노트** (기본 소지) - 메모/힌트 기록
- **RGB 코어** (Level 4-6 획득) - Level 7 진입 필수

**기술 스택:** `Blueprint UMG`

**📖 상세 보기:** [인벤토리 & 노트 →](./System/UI/인벤토리_노트.md)

---

### 🔷 퍼즐 UI (C++ UMG) ⭐

---

#### 베리타스 회로

![베리타스 회로](../Image/System/UI/veritas_ui.png)

**C++ UMG로 구현한 한붓그리기 퍼즐**

NativePaint()를 이용한 실시간 경로 그리기 시스템입니다.

**핵심 기술:**
- `UStrokeGrid` - 메인 위젯 클래스
- `UStrokeCell` - 셀 단위 위젯
- `NativePaint()` - Canvas 직접 그리기

**구현 클래스:**
```cpp
class UStrokeGrid : public UUserWidget
{
    virtual void NativePaint(FPaintContext& Context) const override;
    void UpdateDrawing(FIntPoint NewPos);
};
```

**📖 상세 보기:** [베리타스 회로 →](./System/Puzzle/베리타스_회로.md)

---

#### 타자기 UI

![타자기 UI](../Image/System/UI/typewriter_ui.png)

**키보드 전체 문자 입력 시스템**

Blueprint UMG로 구현한 타자기 입력 UI입니다.

**특징:**
- 키보드 전체 문자 입력 가능
- 실시간 입력 표시
- 정답 시 애니메이션

**📖 상세 보기:** [타자기 시스템 →](./System/Puzzle/타자기_시스템.md)

---

#### 도면도 (힌트)

![도면도](../Image/System/UI/hint_ui.png)

**타일 오픈 방식 힌트 시스템**

정답 도면을 6개 타일로 가리고, 일정 시간마다 타일을 오픈할 수 있습니다.

**작동 방식:**
1. 정답 도면을 6개 타일로 가림
2. 일정 시간마다 타일 1개 오픈 가능
3. 모든 타일 제거 → 정답 확인
4. 레벨 리셋 시 도면도도 리셋

**기술 스택:** `C++ UMG` `HintWidget`

**📖 상세 보기:** [힌트 위젯 →](./System/UI/힌트_위젯.md)

---

### 🔷 메뉴 시스템

![메뉴](../Image/System/UI/menu_main.png)

**게임 메뉴 UI**

메인 메뉴, 일시정지 메뉴, 레벨 선택 화면을 포함합니다.

**구성:**
- **메인 메뉴** - 게임 시작, 레벨 선택, 종료
- **일시정지** (ESC) - 계속하기, 메인으로, 종료
- **레벨 선택** - 해금된 레벨만 선택 가능

**기술 스택:** `Blueprint UMG`

**📖 상세 보기:** [메뉴 시스템 →](./System/UI/메뉴_시스템.md)

---

### 🔷 튜토리얼 위젯

![튜토리얼](../Image/System/UI/tutorial_main.png)

**팝업 메시지 시스템**

특정 트리거 진입 시 팝업으로 표시되는 튜토리얼 메시지입니다.

**특징:**
- 일정 시간 후 자동 사라짐
- 여러 메시지 순차 표시 가능
- 데이터 테이블 기반 관리

**기술 스택:** `C++ UMG` `TutorialWidget` `DataTable`

**📖 상세 보기:** [튜토리얼 위젯 →](./System/UI/튜토리얼_위젯.md)

---

## 🎨 C++ UMG 구현 목록

Harmonia에서 C++ UMG로 구현한 위젯들입니다.

| 클래스 | 용도 | 핵심 기능 |
|--------|------|----------|
| **UStrokeGrid** | 베리타스 회로 | NativePaint() 실시간 그리기 |
| **UStrokeCell** | 베리타스 셀 | 개별 셀 렌더링 |
| **TutorialWidget** | 튜토리얼 팝업 | 메시지 표시 |
| **HintWidget** | 힌트 도면도 | 타일 오픈 시스템 |
| **QuestWidget** | 퀘스트 표시 | 레벨 목표 표시 |

---

## 🎯 UI 표시 계층

```
Layer 100: 튜토리얼 팝업 (최상단)
    ↓
Layer 50: 인벤토리/노트
    ↓
Layer 10: 퍼즐 UI (베리타스, 타자기)
    ↓
Layer 0: 게임 HUD (크로스헤어, 퀘스트)
```

---

## 🎮 입력 모드 전환

UI에 따라 입력 모드를 자동으로 전환합니다.

```cpp
// 일반 게임플레이
SetInputMode(GameOnly)

// 인벤토리 열기
SetInputMode(GameAndUI)

// 퍼즐 UI 열기
SetInputMode(UIOnly)
```

---

## 🔧 Blueprint vs C++ UMG

### C++ UMG로 구현한 경우
```
- 실시간 그리기 필요 (NativePaint)
- 복잡한 로직 (경로 검증)
- 성능 최적화 필요
```

**예시:** 베리타스 회로, 튜토리얼 위젯

---

### Blueprint UMG로 구현한 경우
```
- 일반적인 UI
- 디자이너 작업 필요
- 빠른 프로토타이핑
```

**예시:** 인벤토리, 메뉴, 타자기

---

## 📊 UI 작업 흐름

```
기획 → UMG 디자이너 (레이아웃)
    ↓
C++ 필요? (실시간 그리기 등)
    ├─ Yes → C++ UMG 구현
    └─ No → Blueprint 이벤트 그래프
    ↓
BP로 연결 (Widget 생성/제거)
    ↓
테스트 & 피드백
```

---

## 🔗 관련 문서

### 메인 시스템
- [퍼즐 시스템](./퍼즐_시스템.md)
- [상호작용 시스템](./상호작용_시스템.md)

### UI 상세
- [HUD 시스템](./System/UI/HUD_시스템.md)
- [인벤토리 & 노트](./System/UI/인벤토리_노트.md)
- [메뉴 시스템](./System/UI/메뉴_시스템.md)
- [튜토리얼 위젯](./System/UI/튜토리얼_위젯.md)
- [힌트 위젯](./System/UI/힌트_위젯.md)

### 퍼즐 UI
- [베리타스 회로](./System/Puzzle/베리타스_회로.md)
- [타자기 시스템](./System/Puzzle/타자기_시스템.md)