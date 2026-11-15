#!/usr/bin/env bash
# ==============================================================================
# build_xcframework.sh
# Кросс-компиляция libFinanceCore и упаковка в FinanceCore.xcframework для iOS
# Запускается на macOS (локально или в GitHub Actions CI)
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${ROOT_DIR}/build/xcframework"
OUTPUT_DIR="${ROOT_DIR}/apps/ios-companion/Frameworks"

echo "=== [1/4] Подготовка каталогов ==="
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}/ios-device"
mkdir -p "${BUILD_DIR}/ios-simulator"
mkdir -p "${OUTPUT_DIR}"

# 1. Сборка для физического iPhone (arm64)
echo "=== [2/4] Сборка libFinanceCore для iOS Device (arm64) ==="
cmake -B "${BUILD_DIR}/ios-device" \
  -S "${ROOT_DIR}/core" \
  -G "Xcode" \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="18.0" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build "${BUILD_DIR}/ios-device" --config Release --target FinanceCore

# 2. Сборка для симулятора iOS (arm64 + x86_64)
echo "=== [3/4] Сборка libFinanceCore для iOS Simulator (arm64, x86_64) ==="
cmake -B "${BUILD_DIR}/ios-simulator" \
  -S "${ROOT_DIR}/core" \
  -G "Xcode" \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="18.0" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build "${BUILD_DIR}/ios-simulator" --config Release --target FinanceCore

# 3. Создание XCFramework
echo "=== [4/4] Создание FinanceCore.xcframework ==="
rm -rf "${OUTPUT_DIR}/FinanceCore.xcframework"

xcodebuild -create-xcframework \
  -library "${BUILD_DIR}/ios-device/Release-iphoneos/libFinanceCore.a" \
  -headers "${ROOT_DIR}/core/include" \
  -library "${BUILD_DIR}/ios-simulator/Release-iphonesimulator/libFinanceCore.a" \
  -headers "${ROOT_DIR}/core/include" \
  -output "${OUTPUT_DIR}/FinanceCore.xcframework"

echo "✅ Успешно! Фреймворк создан: ${OUTPUT_DIR}/FinanceCore.xcframework"
