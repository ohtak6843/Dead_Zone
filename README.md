# 🧟 Dead Zone

<p align="center">
  <img src="https://github.com/user-attachments/assets/7b4a6aff-b066-44b5-a024-622018e7d0fe" alt="Dead Zone Title" width="700"/>
</p>

![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?logo=cplusplus&logoColor=white)
![Graphics](https://img.shields.io/badge/Graphics-DirectX%2012-107C10?logo=microsoft&logoColor=white)
![Shader](https://img.shields.io/badge/Shader-HLSL-512BD4)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows&logoColor=white)
![IDE](https://img.shields.io/badge/IDE-Visual%20Studio%202022-5C2D91?logo=visualstudio&logoColor=white)

> 황량한 도시를 무대로 끝없이 몰려오는 좀비 무리에서 살아남아야 하는 **3D 좀비 서바이벌 게임**.
> DirectX 12 기반의 자체 렌더링 파이프라인과 Winsock 기반의 네트워크 모듈을 함께 구현한 게임 프로젝트입니다.


## 📖 프로젝트 소개

**Dead Zone**은 플레이어가 자원을 수집하고, 무기를 활용해 좀비를 처치하며, 점점 거세지는 웨이브 속에서 최대한 오래 생존하는 것을 목표로 하는 **3D 좀비 서바이벌 게임**입니다.

본 프로젝트는 단순히 게임을 만드는 데 그치지 않고, 다음과 같은 학습 목적을 함께 달성하는 것을 지향합니다.

- **DirectX 12 기반의 자체 렌더링 파이프라인** 이해
- **HLSL**을 활용한 셰이더 작성을 통해 그래픽스 파이프라인 전반에 대한 이해 심화
- **Winsock 기반 에코 서버(echoserver)** 를 별도 프로젝트로 분리하여, 클라이언트–서버 구조에서의 네트워크 통신을 직접 구현 및 검증

> 💡 그래픽스 + 네트워크 + 게임 로직을 모두 직접 다루는, **로우 레벨 게임 개발 학습 프로젝트**입니다.


## 👨‍💻 개발자

| 이름 | 역할 | 연락처 / GitHub |
| :--- | :--- | :--- |
| **오현택** | Client | [@ohtak6843](https://github.com/ohtak6843) · `ohtak6843@gmail.com` |
| **박우진** | Client | [@Lucianne0424](https://github.com/Lucianne0424) · `pwj0424@naver.com` |
| **최윤서** | Server | [@cys02001](https://github.com/cys02001) · `jamie0730@naver.com` |


## 📅 개발 기간

| 단계 | 기간 | 내용 |
| :--- | :--- | :--- |
| **기획** | 2024.10 ~ 2025.01 | 게임 컨셉 · 시스템 설계 · 요구사항 정의 |
| **개발** | 2025.03 ~ 2025.08 | 클라이언트 / 서버 / 그래픽스 구현 및 통합 |


## 💻 개발 환경

| 항목 | 내용 |
| :--- | :--- |
| **OS** | Windows 10 / 11 (x64) |
| **IDE** | Visual Studio 2022 (v17.9.x) |
| **빌드 시스템** | MSBuild (`.vcxproj`, `.sln`) |
| **컴파일러** | MSVC (Visual C++) |
| **그래픽스 SDK** | DirectX 12 (Windows SDK 포함) |
| **타깃 플랫폼** | Win32 / x64 (Debug · Release) |

> 솔루션 파일(`Game.sln`)을 Visual Studio 2022에서 열면 `Client`와 `echoserver` 두 개의 프로젝트가 함께 로드됩니다.


## 🛠 기술 스택

### Client (게임 클라이언트)

| 카테고리 | 기술 |
| :--- | :--- |
| Language | **C++**, C |
| Graphics API | **Direct3D 12**, DXGI |
| Shading Language | **HLSL** |
| Platform API | Win32 API |
| Build | MSBuild (`Client.vcxproj`) |

### Server (echoserver)

| 카테고리 | 기술 |
| :--- | :--- |
| Language | **C++** |
| Network | **Winsock2** (TCP 기반 에코 서버) |
| Build | MSBuild (`echoserver.vcxproj`) |

### 외부 라이브러리

라이브러리 파일은 저장소 용량 문제로 **별도로 배포**됩니다. (`Setting.txt` 참고)

> `Library/Lib` 폴더에 별도 배포된 Lib 폴더를 넣어주어야 빌드가 가능합니다.

<!-- TODO: 사용 중인 외부 라이브러리(예: FBX SDK, DirectXTK12, DirectXTex, FMOD 등) 목록과 버전을 명시해 주세요. -->


## ✨ 주요 기능

- **🧟 좀비 서바이벌 게임플레이**
  좀비 무리로부터 살아남는 것을 목표로 하는 3D 액션 서바이벌 게임플레이를 제공합니다.

- **🌐 클라이언트–서버 구조**
  `echoserver` 프로젝트를 통해 **Winsock 기반의 TCP 통신**을 분리 구현하여, 멀티플레이/네트워크 동기화의 기반이 되는 통신 계층을 검증할 수 있습니다.

- **🗂 JsonManager 기반 데이터 Import / Export**
  자체 구현한 **JsonManager**를 통해 게임 데이터(설정, 아이템, 스테이지 정보 등)를 JSON 포맷으로 **Import / Export**할 수 있도록 하여, 코드 수정 없이 외부 파일만으로 게임 데이터를 손쉽게 추가·수정할 수 있습니다.

- **📦 FBX → Bin 파일 변환 및 로딩 파이프라인**
  FBX SDK를 활용해 **FBX 파일을 자체 정의한 Bin 파일로 Export**하고, 런타임에는 가공된 Bin 파일을 직접 **Load**하는 구조를 구축했습니다. 이를 통해 무거운 FBX 파싱 과정을 사전 처리로 분리하여 **로딩 속도와 메모리 효율**을 개선했습니다.

- **🧱 Transform 연동 Collider 구조**
  객체의 **Transform(위치 · 회전 · 스케일) 변화에 따라 자동으로 갱신되는 Collider 시스템**을 구현했습니다. 이를 통해 동적으로 움직이는 오브젝트도 별도의 수동 갱신 없이 정확한 충돌 판정을 수행할 수 있습니다.

<!-- TODO: 인게임 기능(웨이브 시스템, 인벤토리, 무기 종류, 카메라 등) 추가 세부 사항이 있다면 보강해 주세요. -->


## 🖼 스크린샷

### 게임플레이

<p align="center">
  <img src="https://github.com/user-attachments/assets/ff907c99-7339-4fbf-9c5a-75d24d3afb91" alt="Dead Zone Gameplay" width="800"/>
</p>

> 1인칭 시점에서 좀비 웨이브에 대응하는 인게임 플레이 화면. 좌측 하단에는 플레이어들의 점수, 처치 수, 자원, 체력 등의 HUD가 표시되며 우측 하단에는 현재 사용 중인 무기 슬롯이 표시됩니다.


## 📁 폴더 구조

```text
Dead_Zone/
├── Client/          # 게임 클라이언트 (DirectX 12, HLSL, Win32)
├── echoserver/      # Winsock 기반 에코 서버
├── Library/         # 외부 라이브러리 (Lib 폴더는 별도 배포)
├── Resources/       # 텍스처, 메시, 셰이더 등 게임 리소스
├── Game.sln         # Visual Studio 2022 솔루션
├── Setting.txt      # 빌드/세팅 안내
├── .gitattributes
└── .gitignore
```
