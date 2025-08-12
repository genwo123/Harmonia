# 🎭 Harmonia (하모니아)
> **1인칭 퍼즐 어드벤처 게임 - 졸업전시작**

<div align="center">
  <img src="./image/harmonia_banner.png" alt="하모니아 게임 배너" width="80%">
</div>

---

## 📋 **프로젝트 개요**

**하모니아**는 언리얼 엔진 5를 기반으로 개발된 1인칭 퍼즐 어드벤처 게임입니다. 플레이어는 신비로운 세계에서 다양한 퍼즐을 해결하며 깊이 있는 스토리를 경험하게 됩니다.

### **게임 정보**
- **장르**: 1인칭 퍼즐 어드벤처
- **플랫폼**: PC (Windows)
- **엔진**: Unreal Engine 5.1+
- **개발 언어**: C++17, Blueprint Visual Scripting
- **개발 기간**: 2024.03 ~ 2024.11
- **개발 규모**: 7인 팀 (기획 2명, 아트 3명, 사운드 1명, 프로그래밍 1명)

---

## 👥 **협업 가이드**

### 🔗 **[📖 Git & SourceTree 협업 가이드 보기](./COLLABORATION_GUIDE.md)**

**팀원분들을 위한 상세한 협업 가이드입니다:**
- Git 설치 방법
- SourceTree 사용법
- 브랜치 작업 방법
- 파일 업로드 및 관리
- 작업 폴더 구조 안내

---

## 🎮 **게임 특징**

### **몰입감 있는 1인칭 시점**
- Enhanced Input System 기반 현대적 조작감
- 직관적이고 자연스러운 플레이어 컨트롤
- 1인칭 시점에 최적화된 UI/UX

### **창의적 퍼즐 시스템**
- **레이저-거울 퍼즐**: 물리 기반 광선 반사 시뮬레이션
- **일필휘지 퍼즐**: 격자 기반 경로 찾기 게임
- **미로 시스템**: 동적 미로 생성 및 해결
- **키패드 퍼즐**: 논리적 사고를 요구하는 암호 해독

### **깊이 있는 스토리텔링**
- DataTable 기반 동적 대화 시스템
- 플레이어 선택에 따른 스토리 분기
- 몰입감을 높이는 내러티브 연출
- 캐릭터별 고유한 성격과 대화 패턴

### **정교한 상호작용 시스템**
- 인터페이스 패턴 기반 통합 상호작용
- 거리 기반 상호작용 감지
- 동적 상호작용 텍스트 생성
- 직관적인 피드백 시스템

---

## 🛠️ **기술적 특징**

### **현대적 언리얼 엔진 5 기술 활용**
- **Enhanced Input System**: 차세대 입력 처리 시스템
- **Common UI Framework**: 체계적인 UI 관리
- **DataTable System**: 효율적인 게임 데이터 관리
- **Component Architecture**: 확장 가능한 시스템 설계

### **고급 프로그래밍 기법**
- **C++17 모던 문법**: 타입 안전성과 성능 최적화
- **Blueprint 연동**: 생산성과 성능의 완벽한 조화
- **SOLID 원칙**: 유지보수 가능한 코드 아키텍처
- **인터페이스 패턴**: 확장성 있는 시스템 설계

### **최적화된 퍼즐 엔진**
- 실시간 물리 계산 기반 레이저 반사
- 효율적인 격자 알고리즘으로 경로 계산
- 동적 퍼즐 생성 및 검증 시스템
- 메모리 효율적인 퍼즐 상태 관리

---

## 📂 **프로젝트 구조**

```
Harmonia/
├── Source/                    # C++ 소스 코드
│   ├── Harmonia/
│   │   ├── Characters/        # 캐릭터 관련 클래스
│   │   ├── Interactions/      # 상호작용 시스템
│   │   ├── Puzzles/          # 퍼즐 시스템
│   │   └── UI/               # UI 관련 클래스
├── Content/                   # 언리얼 에셋
│   ├── Blueprints/           # 블루프린트 에셋
│   ├── Levels/               # 게임 레벨
│   ├── UI/                   # UI 위젯
│   ├── Hamoni_mesh/          # 3D 모델 및 에셋
│   ├── Materials/            # 머티리얼
│   └── DataTables/           # 게임 데이터
├── Docs/                     # 문서 및 가이드
└── README.md                 # 프로젝트 소개
```

---

## 🎯 **주요 시스템**

### **캐릭터 시스템**
```cpp
// Enhanced Input 기반 현대적 입력 처리
class AHarmoniaCharacter : public ACharacter
{
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* DefaultMappingContext;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* MoveAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* LookAction;
};
```

### **상호작용 시스템**
```cpp
// 인터페이스 기반 확장 가능한 상호작용
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

class HARMONIA_API IInteractableInterface
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void Interact(AActor* Interactor);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    FString GetInteractionText();
};
```

### **퍼즐 시스템**
- **레이저 퍼즐**: 실시간 물리 기반 광선 계산
- **미로 퍼즐**: 동적 생성 및 최적화된 경로 찾기
- **논리 퍼즐**: 조건부 활성화 및 순서 검증

---

## 🏆 **기술적 성과**

### **시스템 아키텍처**
- 컴포넌트 기반 모듈화 설계로 재사용성 극대화
- 인터페이스 패턴으로 확장성 확보
- DataTable 활용으로 기획자 친화적 데이터 관리

### **사용자 경험**
- Enhanced Input System으로 직관적 조작감 구현
- 실시간 피드백 시스템으로 몰입감 향상
- 접근성을 고려한 UI/UX 설계

### **성능 최적화**
- 효율적인 메모리 관리 및 가비지 컬렉션 최적화
- 퍼즐 시스템의 계산 복잡도 최소화
- 렌더링 파이프라인 최적화

---

## 🎮 **플레이 가이드**

### **기본 조작**
- **이동**: WASD 키
- **시점 변경**: 마우스
- **상호작용**: E 키
- **인벤토리**: I 키
- **메뉴**: ESC 키

### **퍼즐 해결 팁**
1. **레이저 퍼즐**: 거울의 각도를 조정하여 목표 지점에 레이저 조준
2. **일필휘지**: 모든 점을 한 번씩 지나가는 경로 찾기
3. **미로**: 단서를 활용하여 올바른 경로 발견

---

## 📸 **스크린샷**

<div align="center">
  <table>
    <tr>
      <td><img src="./image/screenshot1.png" alt="레이저 퍼즐" width="100%"></td>
      <td><img src="./image/screenshot2.png" alt="대화 시스템" width="100%"></td>
    </tr>
    <tr>
      <td align="center"><i>레이저-거울 퍼즐 시스템</i></td>
      <td align="center"><i>동적 대화 시스템</i></td>
    </tr>
  </table>
</div>

---

## 🛠️ **개발 도구**

- **게임 엔진**: Unreal Engine 5.1+
- **IDE**: Visual Studio 2022
- **버전 관리**: Git, SourceTree
- **협업 도구**: Discord, GitHub
- **문서화**: Markdown, GitHub Wiki

---

## 👨‍💻 **개발팀**

### **프로그래밍**
- **김건우** - 메인 프로그래머, 시스템 아키텍트

### **게임 기획**
- **기획자 1** - 게임 디자인, 레벨 디자인
- **기획자 2** - 스토리 기획, 퍼즐 디자인

### **아트**
- **아티스트 1** - 3D 모델링, 환경 아트
- **아티스트 2** - 캐릭터 아트, 애니메이션
- **아티스트 3** - UI/UX 디자인, 이펙트

### **사운드**
- **사운드 디자이너** - 음향 효과, 배경음악

---

## 🔗 **링크**

- **GitHub Repository**: [https://github.com/genwo123/Harmonia](링크)
- **협업 가이드**: [COLLABORATION_GUIDE.md](./COLLABORATION_GUIDE.md)
- **게임플레이 영상**: [YouTube](링크)
- **개발 문서**: [Wiki](링크)

---

## 📄 **라이선스**

이 프로젝트는 숙명여자대학교 2025 졸업전시회 작품입니다

---

<div align="center">
  <i>🎮 하모니아와 함께 퍼즐의 세계를 탐험해보세요! 🎮</i>
</div>