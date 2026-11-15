#!/usr/bin/env python3
"""
bundle_ui.py
Скрипт сборки и проверки статических ассетов web-ui/ для Windows WebView2.
Копирует HTML/CSS/JS в каталог сборки десктопного приложения.
"""

import sys
import shutil
from pathlib import Path

def bundle(src_dir: Path, dest_dir: Path):
    if not src_dir.exists():
        print(f"❌ Ошибка: исходный каталог {src_dir} не существует!")
        sys.exit(1)

    index_file = src_dir / "index.html"
    if not index_file.exists():
        print(f"⚠️ Предупреждение: {index_file} отсутствует в {src_dir}.")

    dest_dir.mkdir(parents=True, exist_ok=True)
    print(f"Копирование ассетов из {src_dir} в {dest_dir}...")

    # Копирование с заменой
    for item in src_dir.iterdir():
        if item.name.startswith("."):
            continue
        dest_item = dest_dir / item.name
        if item.is_dir():
            shutil.copytree(item, dest_item, dirs_exist_ok=True)
        else:
            shutil.copy2(item, dest_item)

    print("✅ Ассеты веб-интерфейса успешно подготовлены для WebView2.")

def main():
    root_dir = Path(__file__).resolve().parent.parent
    src = root_dir / "web-ui"

    if len(sys.argv) > 1:
        dest = Path(sys.argv[1]).resolve()
    else:
        dest = root_dir / "build" / "bin" / "web-ui"

    bundle(src, dest)

if __name__ == "__main__":
    main()
