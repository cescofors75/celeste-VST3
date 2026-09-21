#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
if [ "$(uname -s)" != Darwin ]; then
  echo "This build requires macOS and Apple's developer tools." >&2
  exit 1
fi
if ! xcrun --find clang >/dev/null 2>&1; then
  echo "Install Apple's Command Line Tools (xcode-select --install), then run again." >&2
  exit 1
fi
command -v cmake >/dev/null || { echo "CMake 3.22+ is required (https://cmake.org/download/)." >&2; exit 1; }
deps="$PWD/.mac-deps"
build="$PWD/.mac-build"
out="$PWD/mac-release"
mkdir -p "$deps" "$out"
if [ ! -d "$deps/JUCE/.git" ]; then
  git clone --depth 1 --branch 8.0.12 https://github.com/juce-framework/JUCE.git "$deps/JUCE"
fi
if [ "$(git -C "$deps/JUCE" describe --tags --exact-match)" != 8.0.12 ]; then
  echo "Expected JUCE 8.0.12 in $deps/JUCE" >&2; exit 1
fi
cmake -S . -B "$build" -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DJUCE_SOURCE_DIR="$deps/JUCE"
cmake --build "$build" --config Release --parallel 3 \
  --target Celeste_VST3 Celeste_AU Celeste_Standalone CelesteCheck
"$build/CelesteCheck_artefacts/Release/CelesteCheck" | tee "$out/dsp-tests.txt"
artifacts="$build/Celeste_artefacts/Release"
ditto "$artifacts/VST3/CELESTE Parallel.vst3" "$out/CELESTE Parallel.vst3"
ditto "$artifacts/AU/CELESTE Parallel.component" "$out/CELESTE Parallel.component"
ditto "$artifacts/Standalone/CELESTE Parallel.app" "$out/CELESTE Parallel.app"
for bundle in "$out/CELESTE Parallel.vst3" "$out/CELESTE Parallel.component" "$out/CELESTE Parallel.app"; do
  lipo "$bundle/Contents/MacOS/CELESTE Parallel" -verify_arch arm64 x86_64
done
cp README-MAC.md "$out/LEEME-MAC.md"
mkdir -p "$out/licenses"
cp "$deps/JUCE/LICENSE.md" "$out/licenses/JUCE-8-LICENSE.md"
cp "$deps/JUCE/modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt" "$out/licenses/VST3-LICENSE.txt"
ditto -c -k --sequesterRsrc --keepParent "$out" "$PWD/CELESTE-Parallel-macOS-Universal.zip"
echo "Built Universal VST3 + AU + standalone. Output: $out"
echo "Local development build: not Developer ID signed or notarized. No security settings were changed."
