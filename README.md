<div align="center">

# Harmonia (하모니아)

> **1인칭 레이저 퍼즐 어드벤처 게임**  
> 숙명여자대학교 2025 졸업전시작

<br/>

### 🎬 프로모션 영상
*이미지를 클릭하시면 YouTube PV 영상을 보실 수 있습니다*

[![하모니아 프로모션](./Docs/Image/Harmonia_PV.png)](https://youtu.be/rGCgl6xelgo?si=o7_9UaGb29jtHJu8)

---

## 📋 프로젝트 개요

### 게임 정보

| 항목 | 내용 |
|------|------|
| **장르** | 1인칭 퍼즐 어드벤처 |
| **플랫폼** | PC (Windows) |
| **엔진** | Unreal Engine 5.6 |
| **개발 언어** | C++17, Blueprint |
| **개발 기간** | 2024.03 ~ 2024.11 |
| **플레이 타임** | 1-2시간 (초견 기준) |

### 게임 컨셉

**RGB 색광 혼합을 활용한 레이저 반사 퍼즐**

- 레이저 반사/굴절/변환을 통한 경로 조정
- 그리드 기반 받침대 밀기/회전 시스템
- 한붓그리기 퍼즐로 레이저 활성화
- 타자기 패스워드 입력 등 다양한 서브 퍼즐

### 스토리 (3줄 요약)

1. **태양 접근으로 지구 멸망 위기**
2. **전 연구원 Noah가 연구소로 돌아가 비상 프로그램 작동 시도**
3. **Unia(로봇)가 최종 열쇠 → 희생 vs 탈출 선택**

---

## 👥 개발팀

| 역할 | 인원 | 담당 |
|------|------|------|
| 🖥️ **프로그래밍** | 1명 | 김건우 |
| 📝 **게임 기획** | 1명 | 정영서 |
| 🎨 **아트** | 3명 | 류지원, 박지우, 박소율 |
| 🎵 **사운드** | 1명 | 박세이 |

**총 6인 팀 프로젝트**

---

## 🛠️ 기술 스택

### 개발 환경

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine%205.6-0E1128?style=for-the-badge&logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C++17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Blueprint](https://img.shields.io/badge/Blueprint-0E1128?style=for-the-badge&logo=unrealengine&logoColor=white)

### 협업 도구

![Git](https://img.shields.io/badge/Git-F05032?style=for-the-badge&logo=git&logoColor=white)
![SourceTree](https://img.shields.io/badge/SourceTree-0052CC?style=for-the-badge&logo=sourcetree&logoColor=white)

### 주요 기술 요소

**C++ 구현**
- Enhanced Input System
- C++ UMG (NativePaint 커스텀 그리기)
- Grid System (그리드 기반 배치)
- InteractableMechanism (상호작용 시스템)

**Blueprint 구현**
- 레이저 시스템 (RGB 색광 혼합)
- 퍼즐 메커니즘
- UI/UX 시스템

---

## 📚 문서 바로가기

### 🤝 협업 가이드
| |
|---|
| [Git & SourceTree 협업 가이드](./Docs/COLLABORATION_GUIDE.md) |

---

### 💻 개발 문서

#### 📖 전체 문서
| |
|---|
| [📚 개발 문서 전체 목차](./Docs/MDs/README.md) ⭐ |
| [📝 개발 회고](./Docs/MDs/개발_회고.md) |

#### 🎯 시스템 개요 (메인 문서)

| | | |
|---|---|---|
| [시스템 아키텍처](./Docs/MDs/시스템_아키텍처.md) | [캐릭터 시스템](./Docs/MDs/캐릭터_시스템.md) | [상호작용 시스템](./Docs/MDs/상호작용_시스템.md) |
| [퍼즐 시스템](./Docs/MDs/퍼즐_시스템.md) ⭐ | [UI 시스템](./Docs/MDs/UI_시스템.md) ⭐ | [대화 시스템](./Docs/MDs/대화_시스템.md) |

---

## 🎮 게임 특징

### 🔷 레이저 색광 퍼즐
RGB 가산 혼합 원리를 이용한 색 변환 퍼즐

**핵심 메커니즘:**
- 흰색 레이저 → 컬러박스로 색 변환
- 거울/반사패널로 경로 조정
- 분광기로 2갈래 분리
- 목표 지점에 보색 도달 시 문 열림

---

### 🔷 받침대 시스템
그리드 기반 밀기/회전 메커니즘

**조작 방법:**
- **R키:** 밀기 (상하좌우 그리드 이동)
- **E키:** 45° 회전 (오브젝트와 함께 회전)

---

### 🔷 베리타스 회로
C++ UMG로 구현한 한붓그리기 퍼즐

**특징:**
- NativePaint()를 이용한 실시간 그리기
- 레이저 분출구 활성화 필수 과정
- 레벨별 난이도 상승

---

### 🔷 복합 메커니즘
다양한 서브 퍼즐 시스템

- **타자기:** 숨겨진 단어 찾아 패스워드 입력
- **복도 퍼즐:** 순차 경로 기억 게임
- **도면도:** 힌트 시스템

---

## 📊 개발 현황

| 카테고리 | 진행도 | 비고 |
|---------|--------|------|
| **레벨 디자인** | 90% | Level 0-10 완성 (엔딩 제외) |
| **핵심 시스템** | 95% | 레이저, 받침대, 퍼즐 완성 |
| **UI/UX** | 80% | 기본 UI 완성, 세부 개선 중 |
| **사운드** | 70% | 기본 사운드 완성 |
| **전체** | **약 85%** | 데모 버전 완성 |

---

## 🎯 핵심 성과

✅ **C++ UMG로 실시간 그리기 구현** (베리타스 회로)  
✅ **그리드 시스템 구현** (받침대 밀기/회전)  
✅ **RGB 색광 혼합 퍼즐 설계**  
✅ **데이터 테이블 기반 확장 가능한 레벨 시스템**  
✅ **C++와 Blueprint 혼합 개발**

---

## 📝 라이선스

본 프로젝트는 숙명여자대학교 졸업전시작으로 제작되었습니다.  
모든 저작권은 개발팀에게 있습니다.

---

</div>