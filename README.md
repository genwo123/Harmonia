# 🎮 Harmonia 퍼즐 게임 - 디자이너 협업 가이드

## 📑 **목차**

1. [Git 설치](#1단계-git-설치)
2. [SourceTree 설치](#2단계-sourcetree-설치)
3. [프로젝트 연결](#3단계-프로젝트-연결)
4. [브랜치 작업](#4단계-브랜치-작업)
5. [일상 작업 방법](#5단계-일상-작업-방법)
6. [작업 폴더 안내](#작업-가능한-폴더들)
7. [주의사항 및 FAQ](#주의사항)

---

## 📞 **시작하기 전에**

### **GitHub 계정 알려주세요!**
협업을 위해 **GitHub 계정 아이디**를 프로그래머에게 알려주세요.
- 예: `designer_kim`, `level_artist` 등
- **Collaborator로 초대받으셔야 합니다**

---

## 📥 **1단계: Git 설치**

### **Git 다운로드 및 설치**
> **참고 블로그**: [Git 설치 방법 상세 가이드](https://dev-coco.tistory.com/47)

1. **Git 공식 사이트** 접속: https://git-scm.com/
2. **"Download for Windows"** 클릭
3. **설치 파일 실행** 후 기본 설정으로 설치
4. **설치 완료** (설정은 소스트리에서 자동 처리됩니다)

![Git 사이트](Docs/Image/Git_Site.PNG)
![Git 다운로드](Docs/Image/Git_Site2.PNG)
![Git 설치](Docs/Image/Git_Site3.PNG)

---

## 🛠️ **2단계: SourceTree 설치**

### **SourceTree 다운로드**
1. **SourceTree 공식 사이트** 접속: https://www.sourcetreeapp.com/
2. **"Download for Windows"** 클릭

![SourceTree 사이트](Docs/Image/Sourcetree_Site1.PNG)

### **라이센스 동의**
3. **라이센스 체크박스** 선택
4. **"Download"** 클릭

![라이센스 동의](Docs/Image/Sourcetree_Site2.png)

### **설치 및 초기 설정**

#### **계정 설정 (건너뛰기)**
5. **Bitbucket 계정** 선택 또는 **"건너뛰기"** 클릭

![계정 설정](Docs/Image/Sourcetree_Site3.PNG)

#### **도구 설치**
6. **Git 체크 확인** (이미 설치됨)
7. **Mercurial은 체크 해제**
8. **"다음"** 클릭

![도구 설치](Docs/Image/Sourcetree_Site4.PNG)

#### **사용자 정보 입력**
9. **Author Name**: 본인 이름 입력 (예: `김디자인`)
10. **Author Email**: 본인 이메일 입력
11. **"다음"** 클릭

![사용자 정보](Docs/Image/Sourcetree_Site5.PNG)

#### **SSH 키 설정**
12. **"아니오"** 클릭 (나중에 설정 가능)

![SSH 키](Docs/Image/Sourcetree_Site6.PNG)

---

## 🔗 **3단계: 프로젝트 연결**

### **프로젝트 클론 (복사)**

#### **Clone 시작**
1. **SourceTree** 실행
2. **"Clone"** 버튼 클릭

![Clone 시작](Docs/Image/Sourcetree7-1.PNG)

#### **프로젝트 정보 입력**
3. **소스 경로/URL**: `https://github.com/genwo123/Harmonia.git`
4. **목적지 경로**: 원하는 폴더 선택 (예: `C:\Users\본인이름\OneDrive\바탕화면\Harmonia`)
5. **이름**: `Harmonia`
6. **"클론"** 클릭

![Clone 설정](Docs/Image/Sourcetree7-2.PNG)

#### **클론 완료**
7. **프로젝트 로드 완료** 확인

![클론 완료](Docs/Image/Sourcetree7-3.PNG)

---

## 🌿 **4단계: 브랜치 작업**

### **본인 브랜치로 전환**
> **프로그래머가 미리 생성해둔 브랜치 사용**

1. **좌측 브랜치 목록**에서 **origin** 확장
2. **본인 브랜치** 찾기 (예: `design/김디자인`)
3. **브랜치 더블클릭** → **체크아웃**

**브랜치가 안 보이면:**
- **"Pull"** 버튼 클릭하여 최신 정보 가져오기

### **작업 전 확인사항**
- **"Pull"** 먼저 실행해서 변경사항 확인
- **바뀐 것이 없으면** 안전하게 작업 시작
- **합병이 필요할 때는 프로그래머에게 연락**

---

## 🎯 **5단계: 일상 작업 방법**

### **작업 시작하기**
1. **SourceTree** 열기
2. **"Pull"** 클릭 (최신 상태 유지)
3. **본인 브랜치 확인** (좌측에서 굵은 글씨로 표시)
4. **언리얼 엔진** 열어서 작업

### **작업 완료 후 업로드**

#### **변경사항 확인**
1. **언리얼에서 저장** (Ctrl+S)
2. **SourceTree**로 돌아가기
3. **"파일 상태"** 탭에서 변경된 파일 확인

![파일 상태](Docs/Image/Sourcetree7-4.PNG)

#### **Stage 및 커밋**
4. **변경된 파일들 Stage** (+ 버튼 클릭)
5. **커밋 메시지 작성**:
   ```
   Level_01 퍼즐 배치 완료
   ```
6. **"커밋"** 클릭

![Stage 및 커밋](Docs/Image/Sourcetree7-5.PNG)

#### **서버에 업로드**
7. **"Push"** 클릭 (서버에 업로드)

---

## 📁 **작업 가능한 폴더들**

### **레벨 작업**
- **Content/Level/** - 레벨 생성 및 수정 (Content/Maps는 사용하지 않음)

### **에셋 작업 및 정리**
- **Content/Hamoni_mesh/** - 3D 모델들 및 관련 에셋
  - **폴더별 정리 방식**: 한 오브젝트당 하나의 폴더
  - **예시 구조**:
    ```
    Content/Hamoni_mesh/
    ├── Floor/
    │   ├── SM_Floor.uasset        (스태틱 메시)
    │   ├── M_Floor.uasset         (머티리얼)
    │   └── T_Floor_Diffuse.uasset (텍스처)
    ├── Wall/
    │   ├── SM_Wall.uasset
    │   ├── M_Wall.uasset
    │   └── T_Wall_Normal.uasset
    └── Door/
        ├── SM_Door.uasset
        ├── M_Door.uasset
        └── T_Door_Albedo.uasset
    ```

### **기타 사용 가능한 에셋들**
- **Content/Harmonia/** - 퍼즐 시스템 (레이저, 거울, 키패드)
- **Content/Materials/** - 공용 머티리얼들
- **Content/Image/** - 공용 텍스처들

---

## ⚠️ **주의사항**

### **작업 전 필수사항**
- **반드시 Pull 먼저** 실행
- **본인 브랜치에서만 작업**
- **main 브랜치 직접 수정 금지**

### **커밋 메시지 예시**
```
Level_Tutorial 완성
Stage1 퍼즐 배치 수정
에셋 위치 조정
UI 레이아웃 변경
Floor 에셋 추가 (SM_Floor, M_Floor, T_Floor)
```

### **문제 발생시**
- **충돌 발생**: 즉시 작업 중단 후 프로그래머 연락
- **브랜치 합병 필요시**: 프로그래머에게 연락
- **파일 사라짐**: 브랜치 확인

---

## 🆘 **자주 사용하는 버튼들**

| 버튼 | 설명 |
|------|------|
| **Pull** | 최신 변경사항 가져오기 |
| **Commit** | 내 작업 저장 |
| **Push** | 서버에 업로드 |
| **파일 상태** | 변경된 파일 확인 |

---

## 🎉 **작업 흐름 요약**

1. **SourceTree 열기**
2. **"Pull"** (최신 상태 확인)
3. **본인 브랜치 확인**
4. **언리얼에서 레벨/에셋 작업**
5. **저장 후 SourceTree**
6. **파일 Stage → 커밋 메시지 → Commit**
7. **"Push"** (서버 업로드)
8. **브랜치 합병 필요시 프로그래머에게 연락**

**안전하고 체계적인 협업을 위해 가이드를 따라주세요!** 🚀