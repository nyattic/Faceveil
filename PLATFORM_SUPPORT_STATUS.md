# Redactly 플랫폼 지원 현황 및 남은 작업

작성 기준일: 2026-07-13  
대상 브랜치: `main`  
대상 애플리케이션 버전: `1.9.0`

## 1. 문서 목적

이 문서는 Redactly의 Linux, Windows, macOS 지원 상태와 GPU 가속 구현 범위, 의존성 업그레이드 현황, 검증 결과, 릴리스 전 남은 작업을 기록한다.

다음 세 상태를 구분한다.

- 구현 완료: 필요한 코드 또는 워크플로우 변경이 로컬 작업 트리에 존재한다.
- 로컬 검증 완료: 현재 Linux 개발 환경에서 빌드나 테스트를 통과했다.
- 릴리스 검증 완료: GitHub Actions와 각 운영체제의 실제 하드웨어에서 패키지를 실행해 검증했다.

현재 변경사항은 로컬 작업 트리에 있으며 아직 커밋, 푸시, 원격 GitHub Actions 실행이 완료되지 않았다.

## 2. 전체 상태 요약

| 영역 | 구현 | 로컬 검증 | 원격 또는 실기기 검증 | 현재 판단 |
|---|---:|---:|---:|---|
| Linux CPU 빌드 | 완료 | 완료 | 미완료 | 개발 가능 |
| Linux NVIDIA CUDA 소스 빌드 | 완료 | 부분 완료 | 미완료 | 실제 모델 추론 확인 필요 |
| Linux AMD MIGraphX/ROCm 소스 빌드 | 완료 | 미완료 | 미완료 | AMD 장비 검증 필요 |
| Linux AppImage CPU 추론 | 완료 | 부분 완료 | 미완료 | 패키지 생성 CI 실행 필요 |
| Linux AppImage GPU 추론 | 미완료 | 미완료 | 미완료 | 별도 GPU 배포물 필요 |
| Linux NVENC/Quick Sync 영상 인코딩 | 기존 구현 | 자동 테스트 일부 완료 | 미완료 | 실제 하드웨어 확인 필요 |
| Windows DirectML 추론 | 완료 | 현재 Linux에서는 검증 불가 | 미완료 | Windows CI와 GPU 확인 필요 |
| Windows 패키징 | 완료 | 정적 검사 완료 | 미완료 | 원격 패키지 생성 필요 |
| macOS CoreML 추론 | 기존 구현 | 현재 Linux에서는 검증 불가 | 미완료 | Apple Silicon 확인 필요 |
| macOS DMG 서명 및 공증 | 완료 | 현재 Linux에서는 검증 불가 | 미완료 | 원격 릴리스 작업 필요 |
| 최신 버전 CI 구성 | 완료 | YAML 구문 확인 | 미완료 | GitHub Actions 실행 필요 |

## 3. 의존성 및 빌드 환경 기준

현재 작업 트리의 기준 버전은 다음과 같다.

| 구성 요소 | 기준 버전 | 적용 범위 | 비고 |
|---|---:|---|---|
| Qt | 6.10.3 | Linux, Windows | CMake 최소 버전은 6.8.1 |
| Qt | Homebrew 최신 안정판 | macOS | 워크플로우 실행 시점 버전 사용 |
| OpenCV | 4.13.0 | Linux, Windows | 최소 4.10.0 및 5.x도 지원 |
| OpenCV | Homebrew 최신 안정판 | macOS | 워크플로우 실행 시점 버전 사용 |
| ONNX Runtime | 1.27.1 | Linux | 공식 CPU 바이너리 사용 |
| ONNX Runtime | Homebrew 최신 안정판 | macOS | 최근 CI에서는 1.27.0 설치 |
| ONNX Runtime DirectML | 1.24.4 | Windows | 현재 공개된 DirectML NuGet 최신 버전 |
| DirectML | 1.15.4 | Windows | `DirectML.dll`을 릴리스에 포함 |
| FFmpeg | 8.1.2 | Linux, Windows, macOS | 패키징 스크립트의 고정 바이너리 기준 |
| Ubuntu GitHub Actions | 26.04 | Linux CI 및 AppImage | 현재 공개 프리뷰 이미지 |
| Windows GitHub Actions | Server 2025 | Windows CI 및 ZIP | `windows-2025` 사용 |
| macOS GitHub Actions | macOS 26 ARM64 | macOS CI 및 DMG | `macos-26` 사용 |

관련 상위 프로젝트 자료:

- [ONNX Runtime 1.27.1](https://github.com/microsoft/onnxruntime/releases/tag/v1.27.1)
- [OpenCV 4.13.0](https://github.com/opencv/opencv/releases/tag/4.13.0)
- [ONNX Runtime DirectML 1.24.4](https://www.nuget.org/packages/Microsoft.ML.OnnxRuntime.DirectML/1.24.4)
- [GitHub Actions 러너 이미지 목록](https://github.com/actions/runner-images#available-images)
- [Ubuntu 26.04 Actions 공개 프리뷰 안내](https://github.blog/changelog/2026-06-11-new-runner-images-in-public-preview/)

## 4. GPU 추론 선택과 CPU 폴백

GPU 가속 설정이 활성화된 경우 운영체제에 따라 다음 실행 공급자를 선택한다.

| 운영체제 | 우선순위 | CPU 폴백 |
|---|---|---|
| macOS | CoreML | 공급자 초기화 또는 모델 워밍업 실패 시 CPU |
| Windows | DirectML | 공급자 초기화 또는 모델 워밍업 실패 시 CPU |
| Linux | CUDA, MIGraphX, 구형 ROCm 순서 | 사용할 수 있는 공급자가 없거나 실행에 실패하면 CPU |

얼굴과 번호판 감지기는 GPU 세션을 생성한 뒤 빈 프레임으로 워밍업한다. 공급자가 표시되더라도 모델 그래프를 실제로 실행하지 못하면 해당 감지기를 CPU 세션으로 다시 만든다. 활동 로그에는 얼굴 및 번호판 감지에 사용된 백엔드 이름이 표시된다.

GPU 추론과 영상 하드웨어 인코딩은 서로 다른 기능이다.

- ONNX Runtime 실행 공급자는 얼굴 및 번호판 감지를 가속한다.
- FFmpeg의 NVENC, Quick Sync, VideoToolbox는 결과 영상 인코딩을 가속한다.
- 한쪽이 GPU를 사용한다고 해서 다른 쪽도 자동으로 GPU를 사용하는 것은 아니다.

## 5. Linux 지원 현황

### 5.1 NVIDIA CUDA 소스 빌드

Linux 코드에는 `CUDAExecutionProvider` 감지와 `OrtCUDAProviderOptions` 등록이 구현되어 있다. 현재 Arch Linux 개발 환경에서는 `onnxruntime-opt-cuda 1.27.1`을 사용했다.

현재 환경에서 확인된 ONNX Runtime 실행 공급자는 다음과 같다.

- `CUDAExecutionProvider`
- `DnnlExecutionProvider`
- `CPUExecutionProvider`

CUDA 공급자 옵션을 세션 옵션에 등록하는 단계까지 성공했다. CUDA 공급자 라이브러리의 CUDA, cuDNN, NCCL 의존성도 동적 링커 기준으로 모두 해석됐다.

아직 실제 SCRFD 또는 번호판 ONNX 모델을 CUDA 세션에서 끝까지 실행한 검증은 완료되지 않았다. 현재 실행 환경의 장치 접근 제한으로 `nvidia-smi`가 정상 동작하지 않아, RTX 5080에서의 최종 추론 성공과 VRAM 사용 여부는 애플리케이션을 직접 실행해 확인해야 한다.

Arch Linux 권장 패키지는 다음과 같다.

```bash
yay -S --needed base-devel cmake ninja pkgconf qt6-base qt6-tools qt6-svg \
  opencv onnxruntime-opt-cuda spdlog exiv2 ffmpeg
```

ONNX Runtime 패키지 종류를 변경했다면 기존 CMake 캐시가 이전 라이브러리를 유지하지 않도록 빌드 디렉터리를 새로 구성해야 한다.

### 5.2 AMD MIGraphX와 ROCm

Linux 코드에는 `MIGraphXExecutionProvider`와 구형 `ROCMExecutionProvider` 선택 경로가 구현되어 있다. CUDA가 함께 제공되는 특수 환경에서는 CUDA가 먼저 선택된다.

현재 남은 제한은 다음과 같다.

- 실제 AMD GPU에서 빌드하지 않았다.
- MIGraphX 공급자 옵션 등록과 모델 워밍업을 실행하지 않았다.
- 지원 GPU 세대와 ROCm 버전 조합을 확정하지 않았다.
- AMD GPU용 공식 AppImage를 만들지 않았다.

Arch Linux에서는 지원되는 장비에 한해 `onnxruntime-rocm` 계열 패키지를 사용하도록 문서화했다. NVIDIA용 ONNX Runtime과 ROCm용 ONNX Runtime은 충돌할 수 있으므로 한 종류만 설치해야 한다.

### 5.3 Linux AppImage

현재 릴리스 워크플로우는 공식 CPU 패키지인 `onnxruntime-linux-x64-1.27.1`을 다운로드해 AppImage에 포함한다. 따라서 현재 AppImage의 감지 추론은 CPU 전용이다.

사용자의 시스템에 `onnxruntime-opt-cuda`가 설치되어 있어도 AppImage 내부의 CPU용 `libonnxruntime.so`가 우선 사용될 수 있으므로, 소스 빌드와 동일한 CUDA 동작을 기대하면 안 된다.

권장 배포 전략은 다음과 같다.

1. 범용 CPU AppImage를 계속 제공한다.
2. NVIDIA용 CUDA 13 AppImage를 별도 이름으로 제공한다.
3. AMD는 호환성 표가 확보될 때까지 소스 빌드를 우선 지원한다.

예상 산출물 이름은 다음과 같다.

- `Redactly-<version>-x86_64.AppImage`
- `Redactly-<version>-x86_64-cuda13.AppImage`

CUDA AppImage를 만들 때는 ONNX Runtime CUDA 공급자 라이브러리뿐 아니라 CUDA Runtime, cuDNN, NCCL 등 필요한 공유 라이브러리의 배포 가능 여부, 용량, 드라이버 최소 버전, 라이선스를 확인해야 한다. NVIDIA 커널 드라이버 라이브러리는 AppImage에 포함하지 않고 호스트 시스템의 드라이버를 사용해야 한다.

### 5.4 Linux 영상 인코딩

영상 인코딩에는 기존 NVENC와 Quick Sync 선택 경로가 있으며 실패 시 x264 또는 x265 CPU 인코딩으로 폴백한다. 자동 테스트는 FFmpeg 영상 입출력과 CPU 폴백 동작을 다루지만 실제 NVIDIA 또는 Intel GPU 인코딩은 확인하지 않았다.

다음 항목을 실기기에서 확인해야 한다.

- H.264 NVENC 출력
- HEVC NVENC 출력
- Intel Quick Sync 출력
- GPU 인코더가 없는 시스템의 CPU 폴백
- AppImage에 포함된 FFmpeg 8.1.2가 필요한 인코더를 노출하는지 여부

## 6. Windows 지원 현황

Windows 워크플로우는 다음 기준으로 변경됐다.

- `windows-2025` 러너
- Qt 6.10.3 MSVC 2022 x64 패키지
- OpenCV 4.13.0 공식 Windows 패키지
- ONNX Runtime DirectML 1.24.4
- DirectML 1.15.4
- FFmpeg 8.1.2

OpenCV 설정 경로는 특정 `vc16` 디렉터리에 고정하지 않고 압축 해제 결과에서 `OpenCVConfig.cmake`를 검색한다. 패키징 스크립트는 `onnxruntime.dll`과 `DirectML.dll`을 ZIP에 포함하며, `DirectML.dll`을 찾지 못하면 릴리스를 실패시킨다.

ONNX Runtime 핵심 최신 버전은 1.27.1이지만 Windows DirectML NuGet 패키지는 1.24.4가 최신이므로 Windows만 1.24.4를 사용한다. DirectML 패키지가 1.27 계열로 공개되면 별도로 올려야 한다.

남은 검증은 다음과 같다.

- Windows Server 2025 러너에서 Qt, OpenCV, vcpkg 조합 빌드
- DirectML 공급자 감지
- NVIDIA, AMD 또는 Intel GPU에서 얼굴 모델 워밍업과 추론
- DirectML을 사용할 수 없는 환경의 CPU 폴백
- ZIP 압축 해제 후 깨끗한 Windows 환경에서 실행
- H.264와 HEVC 하드웨어 인코딩 및 CPU 폴백

`windows-2025` 이미지의 Visual Studio 도구 구성이 갱신될 수 있으므로 MSVC 2022용 Qt 바이너리 및 vcpkg 결과와의 호환성은 실제 Actions 실행으로 확인해야 한다.

## 7. macOS 지원 현황

macOS 워크플로우는 `macos-26` ARM64 러너를 사용한다. Qt, OpenCV, ONNX Runtime, spdlog, Exiv2는 특정 버전을 별도로 고정하지 않고 워크플로우 실행 시점의 Homebrew 최신 안정판을 설치한다. 최근 CI에서는 Qt 6.11.1, OpenCV 4.13.0, ONNX Runtime 1.27.0이 설치됐다.

CoreML 실행 공급자 선택, 모델 워밍업, CPU 폴백은 기존에 구현되어 있다. 패키징 스크립트는 Apple Silicon DMG를 만들고 애플리케이션과 포함 라이브러리, FFmpeg 도구를 서명한 뒤 릴리스 워크플로우에서 공증과 스테이플링을 수행한다.

남은 검증은 다음과 같다.

- macOS 26 ARM64에서 전체 빌드와 테스트
- Homebrew ONNX Runtime의 CoreML 공급자 포함 여부
- Fast 및 Accurate 얼굴 모델의 CoreML 워밍업
- 번호판 모델의 CoreML 워밍업 또는 CPU 폴백
- VideoToolbox H.264와 HEVC 인코딩
- Developer ID 서명
- Apple 공증과 DMG 스테이플링
- 깨끗한 Apple Silicon Mac에서 DMG 설치 및 실행

현재 릴리스는 Apple Silicon ARM64만 대상으로 한다. Intel macOS용 DMG가 필요하다면 별도 `macos-26-intel` 작업과 x86_64 의존성, 패키지 이름, 테스트 범위를 추가해야 한다.

## 8. CI 및 릴리스 워크플로우 변경

다음 변경이 로컬 작업 트리에 반영되어 있다.

- 세 운영체제의 최신 러너 라벨 적용
- Linux와 Windows Qt 6.10.3 환경 변수화, CMake 최소 버전 6.8.1 적용
- Linux와 Windows OpenCV 4.13.0 적용, macOS는 Homebrew 최신 안정판 사용
- Linux ONNX Runtime 1.27.1 적용
- Windows DirectML 패키지 1.24.4 및 DirectML 1.15.4 적용
- 다운로드 파일 SHA-256 검증
- Linux에서 OpenCV 4.13.0 최소 모듈 소스 빌드
- Windows OpenCV 설정 경로 자동 검색
- Windows와 macOS, Linux FFmpeg 8.1.2 기준 적용

Ubuntu 26.04는 GitHub Actions 공개 프리뷰다. 라벨은 사용할 수 있지만 GA 이미지와 같은 안정성 보장을 기대할 수 없다. 또한 Ubuntu 26.04에서 만든 AppImage는 최신 glibc에 연결되어 이전 Linux 배포판에서 실행되지 않을 수 있다. 최신 LTS만 지원할지, 더 넓은 배포판 호환성을 위해 패키징 기준 이미지를 낮출지는 별도 제품 정책으로 결정해야 한다.

## 9. 완료된 로컬 검증

현재 Linux 개발 환경에서 완료한 검증은 다음과 같다.

- Qt 6.11.1, OpenCV 5.0.0, ONNX Runtime 1.27.1 조합 구성으로 상위 버전 호환성 확인
- OpenCV 5.0.0 소스 최소 빌드 및 설치로 상위 버전 호환성 확인
- 공식 빌드 기준 OpenCV 4.13.0 전환 후 원격 CI 검증 필요
- `onnxruntime-opt-cuda 1.27.1` 설치 후 깨끗한 재빌드
- CUDA 공급자 라이브러리 의존성 해석 확인
- ONNX Runtime 실행 공급자 목록에서 CUDA 확인
- CUDA 공급자 옵션 등록 성공
- CMake 빌드 성공
- 전체 테스트 6개 통과
- GitHub Actions YAML 구문 분석 성공
- 다운로드 대상 패키지 SHA-256 확인
- `git diff --check` 통과
- 프로젝트 소스, 테스트, 도구에 `std::cout` 또는 `std::cerr` 추가 없음

최근 테스트 결과는 다음과 같다.

```text
translation_quality            Passed
translation_quality_zh_CN      Passed
redactly_tests                 Passed
redactly_tracking_tests        Passed
redactly_parallel_tests        Passed
redactly_video_io_tests        Passed
100% tests passed, 0 tests failed out of 6
```

이 결과는 Linux CPU 빌드와 코드 수준 회귀가 없다는 근거다. CUDA 공급자를 사용한 실제 모델 결과의 정확성이나 처리 속도를 보장하는 결과는 아니다.

## 10. 남은 작업 우선순위

### P0: 현재 변경의 기본 검증

#### P0-1. 변경사항 커밋과 CI 실행

완료 조건:

- 변경사항을 브랜치에 커밋하고 GitHub에 푸시한다.
- Linux, Windows, macOS CI가 모두 통과한다.
- 실패한 플랫폼은 로그와 수정 내용을 이 문서에 기록한다.

#### P0-2. RTX 5080 실제 CUDA 추론

완료 조건:

- Redactly를 `onnxruntime-opt-cuda`에 연결해 실행한다.
- Fast 얼굴 모델로 이미지 한 장을 처리한다.
- Accurate 얼굴 모델로 이미지 또는 짧은 영상을 처리한다.
- 번호판 모델로 이미지 한 장을 처리한다.
- 활동 로그에서 각 감지 백엔드가 `CUDA`로 표시되는지 확인한다.
- 별도 터미널에서 VRAM 또는 GPU 사용량 증가를 확인한다.
- 오류가 발생하면 CPU 폴백 여부와 spdlog 메시지를 기록한다.

#### P0-3. Linux 릴리스 GPU 정책 결정

완료 조건:

- CPU AppImage와 CUDA AppImage를 분리할지 결정한다.
- CUDA AppImage가 요구할 최소 NVIDIA 드라이버와 CUDA 13 런타임 정책을 정한다.
- AMD AppImage를 현재 릴리스 범위에 포함할지 결정한다.
- README와 릴리스 설명의 지원 범위를 실제 패키지 동작과 일치시킨다.

### P1: GPU 릴리스 지원

#### P1-1. NVIDIA CUDA 13 AppImage 제작

완료 조건:

- CUDA 공급자가 포함된 ONNX Runtime 1.27.1로 별도 AppImage를 만든다.
- 필요한 사용자 공간 공유 라이브러리를 누락 없이 처리한다.
- NVIDIA 드라이버 라이브러리는 호스트 시스템에서 사용한다.
- AppImage 내부 실행 공급자 목록에 CUDA가 나타난다.
- 실제 NVIDIA GPU에서 얼굴과 번호판 추론이 성공한다.
- CPU 전용 시스템에서는 오류 메시지가 명확하거나 CPU AppImage 사용을 안내한다.
- 라이선스 고지와 배포 크기를 검토한다.

#### P1-2. AMD GPU 검증

완료 조건:

- 지원 대상 AMD GPU와 ROCm 버전을 정한다.
- `onnxruntime-rocm` 또는 MIGraphX 빌드로 프로젝트를 구성한다.
- 실행 공급자 감지, 모델 워밍업, 이미지와 영상 추론을 확인한다.
- 실패 시 CPU 폴백을 확인한다.
- 검증된 하드웨어 조합을 README에 기록한다.

#### P1-3. 플랫폼별 패키지 스모크 테스트

완료 조건:

- Linux AppImage를 빌드 머신과 다른 깨끗한 시스템에서 실행한다.
- Windows ZIP을 빌드 도구가 없는 깨끗한 시스템에서 실행한다.
- macOS DMG를 빌드 머신과 다른 Apple Silicon Mac에서 실행한다.
- 첫 모델 다운로드, 해시 검증, 이미지 처리, 영상 처리, CPU 폴백을 확인한다.

### P2: 장기 유지보수

- GPU가 있는 자체 호스팅 러너 또는 정기 수동 테스트 절차를 마련한다.
- 의존성 최신 버전과 SHA-256 갱신을 자동화한다.
- CPU와 GPU 패키지 파일명 및 릴리스 설명을 자동 생성한다.
- Ubuntu 26.04 프리뷰가 GA로 전환되는 시점을 추적한다.
- 이전 Linux 배포판 지원 범위와 glibc 최소 버전을 명시한다.
- Linux ARM64, Windows ARM64, Intel macOS 지원 필요성을 별도로 평가한다.
- 실제 모델별 CPU, CUDA, DirectML, CoreML 성능 기준을 기록한다.

## 11. 수동 검증 체크리스트

### Linux NVIDIA

- [ ] `pkg-config --modversion libonnxruntime`이 `1.27.1`을 표시한다.
- [ ] CUDA 공급자 공유 라이브러리의 의존성이 모두 해석된다.
- [ ] Redactly 로그에서 얼굴 감지 백엔드가 `CUDA`로 표시된다.
- [ ] Redactly 로그에서 번호판 감지 백엔드가 `CUDA`로 표시된다.
- [ ] 이미지 출력이 정상이며 감지 결과가 CPU 결과와 크게 다르지 않다.
- [ ] 짧은 영상의 감지와 NVENC 인코딩이 모두 성공한다.
- [ ] GPU 설정을 끄면 CPU 백엔드로 실행된다.

### Windows

- [ ] ZIP에 `onnxruntime.dll`과 `DirectML.dll`이 존재한다.
- [ ] 얼굴 및 번호판 백엔드가 `DirectML`로 표시된다.
- [ ] DirectX 12 GPU에서 추론이 성공한다.
- [ ] DirectML 초기화 실패 시 CPU로 폴백한다.
- [ ] H.264 및 HEVC 출력이 재생된다.

### macOS

- [ ] DMG 서명과 공증 검증이 통과한다.
- [ ] 얼굴 감지 백엔드가 `CoreML`로 표시된다.
- [ ] 지원되지 않는 모델 그래프는 CPU로 폴백한다.
- [ ] VideoToolbox 출력이 정상 재생된다.
- [ ] Gatekeeper 경고 없이 설치 및 실행된다.

## 12. 릴리스 준비 완료 기준

다음 조건을 모두 충족해야 플랫폼 업그레이드 작업을 릴리스 준비 완료로 판단한다.

- Linux, Windows, macOS CI 성공
- 세 플랫폼 패키지 생성 성공
- Windows DirectML 실제 추론 성공
- macOS CoreML 실제 추론 성공
- Linux 소스 빌드 CUDA 실제 추론 성공
- Linux AppImage의 GPU 지원 범위를 문서와 일치시킴
- 각 플랫폼에서 CPU 폴백 확인
- 패키지에 필요한 런타임 라이브러리와 라이선스 고지 포함
- 깨끗한 시스템에서 설치 또는 압축 해제 후 기본 처리 성공
- 릴리스 노트와 README 갱신

## 13. 관련 변경 파일

- `.github/workflows/ci.yml`
- `.github/workflows/release.yml`
- `CMakeLists.txt`
- `README.md`
- `include/redactly/OrtAcceleration.hpp`
- `src/OrtAcceleration.cpp`
- `src/Mosaic.cpp`
- `scripts/package_windows.ps1`
- `tests/test_core.cpp`

이 문서는 검증이 진행될 때마다 표의 상태와 완료 조건을 갱신한다.
