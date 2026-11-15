<#
.SYNOPSIS
    Форматирование всех C++ исходников ядра и хоста с помощью clang-format.
#>

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir = Split-Path -Parent $scriptDir

Write-Host "Проверка наличия clang-format..." -ForegroundColor Cyan

$clangFormat = Get-Command clang-format -ErrorAction SilentlyContinue

if (-not $clangFormat) {
    Write-Warning "clang-format не найден в PATH. Установите LLVM / Clang или добавьте его в системные переменные."
    exit 0
}

Write-Host "Форматирование исходного кода в core/ и apps/desktop-win/..." -ForegroundColor Green

$extensions = @("*.hpp", "*.cpp", "*.h", "*.c")
$targets = @(
    Join-Path $rootDir "core",
    Join-Path $rootDir "apps\desktop-win"
)

$count = 0
foreach ($t in $targets) {
    if (Test-Path $t) {
        foreach ($ext in $extensions) {
            Get-ChildItem -Path $t -Filter $ext -Recurse -File | ForEach-Object {
                clang-format -i $_.FullName
                $count++
            }
        }
    }
}

Write-Host "✅ Отформатировано файлов: $count" -ForegroundColor Green
