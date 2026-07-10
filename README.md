# Redactly

**한국어** | [English](README.en.md)

![Release](https://img.shields.io/github/v/release/nyattic/Redactly?style=flat&color=6366f1)
![Downloads](https://img.shields.io/github/downloads/nyattic/Redactly/total?style=flat&color=10b981)
![Last Commit](https://img.shields.io/github/last-commit/nyattic/Redactly?style=flat&color=f59e0b)
![License](https://img.shields.io/badge/license-GPL--3.0--or--later-8b5cf6?style=flat)

사진과 영상 속 얼굴·번호판을 자동으로 찾아 가려주는 로컬 데스크톱 앱입니다. 이미지, 동영상 또는 폴더를 추가하고 탐지 대상을 선택하면 익명화된 사본을 만들 수 있습니다. 모든 파일은 사용자의 기기 안에서만 처리되며 업로드되지 않습니다.

## 설치

[Releases](https://github.com/nyattic/Redactly/releases/latest)에서 OS에 맞는 파일을 다운로드하세요.

- **macOS** (Apple Silicon, macOS 15 이상) — `.dmg`를 열고 Applications 폴더로 드래그
- **Windows** (x64, Windows 10 이상) — 압축을 풀고 `Redactly.exe` 실행. GPU 가속에는 Windows 10 1903 이상과 DirectX 12 지원 GPU(NVIDIA, AMD 또는 Intel)가 필요하며, 사용할 수 없으면 CPU로 탐지합니다.
- **Linux** (x86_64) — `.AppImage`를 다운로드하고 `chmod +x`를 적용한 뒤 실행

기본 모델을 처음 사용할 때 한 번 다운로드(3~17MB)하여 캐시에 저장하며, 이후에는 오프라인으로 작동합니다. 얼굴 모델은 Hugging Face에서, 번호판 모델은 GitHub의 open-image-models 프로젝트에서 받습니다.

## 사용 방법

1. 이미지, 동영상 또는 폴더를 창에 드롭합니다.
2. 탐지 대상에서 **얼굴**, **번호판** 또는 **둘 다**를 선택합니다.
3. 얼굴 탐지 모델에서 **빠름** 또는 **정확**을 선택합니다.
4. 출력 폴더를 지정합니다.
5. **시작**을 누릅니다.

탐지된 영역은 모자이크, 가우시안 블러, 단색 채우기 또는 기본 스마일 스티커로 가릴 수 있습니다. 스티커는 운영체제 이모지 글꼴에 의존하지 않고 앱에서 직접 렌더링하므로 모든 플랫폼에서 같은 모양으로 표시됩니다.

원본 파일은 절대 수정하지 않습니다. **저장 전 검토**를 켜면 이미지의 탐지 상자를 확인하고 놓친 영역을 추가할 수 있으며, 동영상은 인코딩 전에 트랙 타임라인을 검토할 수 있습니다. 동영상 오탐은 해당 트랙 전체에서 한 번에 제외할 수 있습니다.

두 입력이 같은 출력 경로를 만들거나 예정된 출력 파일이 이미 존재하면 작업을 시작하지 않습니다. 같은 일괄 작업을 다시 실행하려면 기존 결과물을 이동하거나 이름을 변경하세요.

모든 항목이 정상적으로 가림 처리되면 작업 상태가 **완료**로 표시됩니다. 실패하거나 건너뛴 파일 또는 탐지 영역 없이 저장된 파일이 있으면 **검토 필요** 상태와 요약을 표시합니다. 이 경우 작업이 완전하지 않은 것으로 간주하고 활동 로그와 결과물을 확인한 뒤 공유하세요.

지원 형식은 `.jpg` `.jpeg` `.png` `.bmp` `.tif` `.tiff` `.webp` 이미지와 `.mp4` `.mov` `.m4v` 동영상(H.264/HEVC, 8비트 SDR)입니다. 동영상 기능은 현재 **베타**이므로 공유하기 전에 결과를 확인하세요. Linux 영상 파이프라인은 자동화 테스트를 통과하지만 아직 수동 검증되지는 않았습니다.

지원되는 환경에서는 GPU로 탐지합니다. macOS는 CoreML, Windows는 DirectML을 사용하며 Windows 릴리스에는 NVIDIA·AMD·Intel GPU를 모두 지원하는 DirectML이 포함됩니다. GPU를 사용할 수 없으면 자동으로 CPU로 전환하며, 설정에서 GPU 가속을 끌 수도 있습니다.

동영상은 양방향 추적을 포함한 탐지와 인코딩의 두 단계로 처리하여 모션 블러나 짧은 가림 구간에도 얼굴을 계속 가립니다. 검토를 켜면 두 단계 사이에서 멈추고 트랙 타임라인을 표시합니다. 인코딩은 가능한 경우 Windows와 Linux에서는 NVENC 또는 Quick Sync, macOS에서는 VideoToolbox 하드웨어 인코더를 사용하며, 실패하거나 GPU 가속을 끄면 CPU x264/x265로 전환합니다. 출력은 H.264(기본) 또는 HEVC MP4이며 설정에서 선택할 수 있고, 원본 오디오를 유지합니다. 원본 오디오 코덱이 MP4와 호환되지 않을 때만 AAC로 다시 인코딩합니다. 컨테이너 메타데이터는 제거하고 회전 정보는 픽셀에 적용합니다. 가변 프레임레이트 입력은 고정 프레임레이트로 변환하며 10비트/HDR 영상은 품질 저하 없이 거부합니다. 영상 처리는 앱과 함께 제공되는 FFmpeg를 우선 사용하고, 없으면 `PATH`에서 `ffmpeg`와 `ffprobe`를 찾습니다. 영상 품질 프리셋은 설정에서 선택할 수 있습니다.

## 소스에서 빌드

CMake 3.24 이상, C++20 컴파일러, Qt 6, OpenCV 4, ONNX Runtime, spdlog가 필요합니다. UI 번역을 포함하려면 Qt Linguist 도구가 필요하고, Qt Svg는 선택 사항입니다. Exiv2는 메타데이터 보존에 사용하는 선택 의존성입니다. FFmpeg는 빌드 의존성이 아니지만 영상 처리 시 실행 가능한 `ffmpeg`와 `ffprobe`가 필요합니다. 얼굴용 SCRFD와 번호판용 YOLOv9 모델은 빌드에 필요하지 않으며 앱이 처음 사용할 때 다운로드합니다.

기본 모델은 앱이나 저장소에 **포함되지 않습니다**. 앱이 처음 사용할 때 무결성을 확인하여 플랫폼 데이터 디렉터리의 캐시에 저장합니다. 오프라인 사용을 위해 미리 배치하려면 다음 파일을 `models/`에 넣으세요.

- `models/2.5g_bnkps.onnx` — 빠름(얼굴)
- `models/10g_bnkps.onnx` — 정확(얼굴)
- `models/yolo-v9-t-512-license-plates-end2end.onnx` — 번호판

앱에서 **찾아보기…**를 눌러 커스텀 SCRFD `.onnx` 파일을 선택할 수도 있습니다.

신뢰할 수 있는 출처의 커스텀 ONNX 모델만 불러오세요. Redactly는 처리 전에 기본적인 SCRFD 텐서 호환성을 확인하지만, ONNX 파일은 네이티브 런타임 라이브러리가 실행하는 모델 입력입니다.

### macOS

```bash
cmake -S . -B build
cmake --build build
open build/Redactly.app
```

Homebrew로 의존성을 설치할 수 있습니다.

```bash
brew install cmake qt opencv onnxruntime spdlog exiv2
```

### Windows (PowerShell)

```powershell
cmake -S . -B build-windows -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.11.0\msvc2022_64;C:\opencv\build" `
  -DONNXRUNTIME_ROOT="C:\onnxruntime-win-x64-1.20.1"
cmake --build build-windows --config Release
```

spdlog도 CMake에서 찾을 수 있어야 합니다. 예를 들어 vcpkg로 설치(`vcpkg install spdlog`)하고 vcpkg 경로를 `CMAKE_PREFIX_PATH`에 추가할 수 있습니다.

개발에는 일반 ONNX Runtime 빌드를 사용할 수 있지만 공식 Windows 릴리스는 GPU 탐지를 위해 DirectML 빌드를 사용합니다. `Microsoft.ML.OnnxRuntime.DirectML` NuGet 패키지를 `include`/`lib` 구조로 배치하고, `Microsoft.AI.DirectML` 패키지의 `DirectML.dll`을 `onnxruntime.dll` 옆에 두어야 합니다. 자세한 구성은 `.github/workflows/release.yml`의 Windows 작업을 참고하세요. `scripts/package_windows.ps1`은 `DirectML.dll`이 없으면 패키징을 중단합니다.

### Linux

Debian/Ubuntu 예시:

```bash
sudo apt install cmake ninja-build build-essential pkg-config \
  qt6-base-dev qt6-tools-dev libqt6svg6-dev libopencv-dev libspdlog-dev libexiv2-dev
```

ONNX Runtime은 가능한 경우 `pkg-config libonnxruntime`으로 찾습니다. 찾을 수 없다면 ONNX Runtime 릴리스 경로를 직접 지정하세요.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DONNXRUNTIME_ROOT=/path/to/onnxruntime-linux-x64
cmake --build build
./build/Redactly
```

### 테스트

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

영상 I/O 테스트는 실제 FFmpeg 디코딩·인코딩 round-trip을 실행하며 FFmpeg가 설치되어 있지 않으면 건너뜁니다.

저장소 밖의 `build*/` CMake 디렉터리, 로컬 ONNX 모델, 생성된 출력물, 패키지 결과물, 미완료 다운로드, 로그 및 일반 IDE 파일은 Git에서 제외됩니다. 소스, 워크플로, 번역, 에셋과 문서는 추적됩니다.

패키징 스크립트: [`scripts/package_macos.sh`](scripts/package_macos.sh), [`scripts/package_windows.ps1`](scripts/package_windows.ps1), [`scripts/package_linux.sh`](scripts/package_linux.sh), [`scripts/notarize_macos.sh`](scripts/notarize_macos.sh)

## 개인정보 보호

이미지와 동영상은 기기 밖으로 전송되지 않습니다. 디스크에서 읽어 로컬에서 처리하고, 선택한 출력 폴더에 저장합니다. 영상 인코딩도 로컬 FFmpeg 프로세스로 실행합니다.

Redactly의 네트워크 요청은 두 종류뿐이며 이미지나 개인정보를 전송하지 않습니다. 기본 탐지 모델을 처음 사용할 때 한 번 다운로드하고, 앱 시작 시 GitHub Releases API에서 새 버전을 확인합니다. 얼굴 모델은 Hugging Face에서, 번호판 모델은 open-image-models 프로젝트에서 받습니다. 업데이트 확인은 **설정 → 시작할 때 업데이트 확인**에서 끌 수 있으며, **찾아보기…**로 커스텀 모델을 지정하면 기본 모델 다운로드를 피할 수 있습니다.

## 라이선스

**애플리케이션 소스 코드** — GNU General Public License v3.0 이상, SPDX 식별자 `GPL-3.0-or-later`. Redactly는 상업적 목적을 포함하여 사용·학습·공유·수정할 수 있습니다. 애플리케이션이나 파생물을 배포할 경우 GPL에 따라 해당 소스 코드를 함께 제공해야 합니다. 자세한 내용은 [LICENSE](LICENSE)를 참고하세요.

> 이 애플리케이션은 과거 PolyForm Noncommercial 1.0.0으로 배포되었습니다. 메타데이터 보존을 위해 [Exiv2](https://exiv2.org/)(GPL-2.0-or-later)를 연결하면서 v1.1.0부터 GPL v3.0 이상으로 전환했습니다. 이전에 출시된 버전은 계속 PolyForm Noncommercial 라이선스를 따릅니다.

Copyright © 2026 Nyabi.

Redactly는 자유 소프트웨어이며 GNU General Public License 버전 3 또는 그 이후 버전의 조건에 따라 재배포하거나 수정할 수 있습니다. 상품성이나 특정 목적에의 적합성을 포함한 어떠한 보증도 제공하지 않습니다.

**SCRFD 모델** — 기본 얼굴 모델은 Redactly와 함께 배포되지 않으며, 처음 사용할 때 [Hugging Face 미러](https://huggingface.co/RuteNL/SCRFD-face-detection-ONNX)에서 다운로드합니다. upstream은 [InsightFace](https://github.com/deepinsight/insightface)이며 애플리케이션과 별개의 조건에 따라 **비상업적 연구 용도로만** 제공됩니다. 미러의 Apache-2.0 표시는 InsightFace의 모델 조건을 대체하지 않습니다. 자세한 내용은 [InsightFace Model Zoo](https://github.com/deepinsight/insightface/blob/master/model_zoo/README.md)를 확인하세요.

**번호판 모델** — 기본 번호판 모델은 Redactly와 함께 배포되지 않으며, 처음 사용할 때 ankandrew의 [open-image-models](https://github.com/ankandrew/open-image-models) 프로젝트에서 다운로드합니다. 이 프로젝트는 MIT 라이선스를 사용하며 YOLOv9 구조의 모델입니다. 자세한 출처는 [인용](#인용)을 참고하고, 상업적 사용이나 재배포 전에는 upstream 프로젝트의 최신 조건을 확인하세요.

**서드파티 런타임 의존성** — Qt(LGPL-3.0/GPL-3.0/상용), OpenCV(Apache-2.0), ONNX Runtime(MIT), DirectML(Microsoft 독점 라이선스, Windows 릴리스의 `DirectML.dll`로만 포함), Exiv2(GPL-2.0-or-later)와 그 의존성(Brotli, Expat, inih, zlib, GNU gettext), spdlog와 {fmt}(MIT), 별도 프로세스로 실행되는 FFmpeg(LGPL-2.1-or-later 및 선택적 GPL 구성 요소)는 각각의 라이선스를 유지합니다. 전체 고지문은 [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt)에 있으며 각 릴리스에도 포함됩니다.

## 인용

```bibtex
@misc{guo2021sample,
  title={Sample and Computation Redistribution for Efficient Face Detection},
  author={Jia Guo and Jiankang Deng and Alexandros Lattas and Stefanos Zafeiriou},
  year={2021},
  eprint={2105.04714},
  archivePrefix={arXiv},
  primaryClass={cs.CV}
}

@misc{wang2024yolov9,
  title={YOLOv9: Learning What You Want to Learn Using Programmable Gradient Information},
  author={Chien-Yao Wang and Hong-Yuan Mark Liao},
  year={2024},
  eprint={2402.13616},
  archivePrefix={arXiv},
  primaryClass={cs.CV}
}
```
