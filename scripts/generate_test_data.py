#!/usr/bin/env python3
"""
generate_test_data.py
Генератор синтетических выписок белорусских банков для тестирования парсеров и дедупликатора.
Не содержит реальных персональных данных.
"""

import os
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent
OUTPUT_DIR = BASE_DIR / "sample_statements"
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

# 1. Приорбанк (Priorbank) — типичная выписка с кодировкой Windows-1251 и разделителем ';'
PRIORBANK_CSV = """Дата операции;Дата проводки;Номер карты;Сумма в валюте операции;Валюта операции;Сумма в валюте счета;Валюта счета;Детали операции
15.11.2026 14:20:00;16.11.2026;*1234;-18.45;BYN;-18.45;BYN;OOO "EVROOPT" G.MINSK BLR
15.11.2026 12:05:12;16.11.2026;*1234;-6.30;BYN;-6.30;BYN;DOSTAJECHKA MINSK BLR
14.11.2026 19:30:00;15.11.2026;*1234;-12.50;BYN;-12.50;BYN;YANDEX.GO MINSK BLR
10.11.2026 10:00:00;10.11.2026;*1234;2500.00;BYN;2500.00;BYN;ZARABOTNAYA PLATA PO DOGOVORU
08.11.2026 09:15:00;08.11.2026;*1234;-5.00;BYN;-5.00;BYN;KOMISSIYA ZA OBSLUZHIVANIE KARTY
"""

# 2. Беларусбанк (Belarusbank) — кодировка Windows-1251, запятая как разделитель
BELARUSBANK_CSV = """Дата операции,Время,Сумма,Валюта,Сумма в валюте счета,Валюта счета,Место совершения операции
15.11.2026,16:45:10,-24.80,BYN,-24.80,BYN,MAGAZIN SODRUGESTVO MINSK
14.11.2026,08:30:22,-3.50,BYN,-3.50,BYN,MINSKTRANS PROEZD
12.11.2026,11:12:00,-150.00,BYN,-150.00,BYN,ERIP PLATELSHIK VZNOS
"""

# 3. МТБанк (MTBank / Moby) — кодировка UTF-8, точка с запятой
MTBANK_CSV = """Дата/время операции;Сумма в валюте операции;Валюта операции;Сумма в валюте карты;Валюта карты;Статус;Описание операции
15.11.2026 18:02:15;-9.99;BYN;-9.99;BYN;Успешно;WILDBERRIES MINSK BY
15.11.2026 13:10:45;-4.50;BYN;-4.50;BYN;Успешно;KOFEYNYA ZERNO
13.11.2026 20:40:00;-35.00;USD;-117.25;BYN;Успешно;GITHUB SUBSCRIPTION SAN FRANCISCO US
"""

# 4. Альфа-Банк (Alfa-Bank / InSync) — кодировка UTF-8, разделитель запятая
ALFABANK_CSV = """Дата операции,Сумма,Валюта,Сумма в валюте счета,Валюта счета,Описание,Категория
15.11.2026,-15.40,BYN,-15.40,BYN,Sosedi Express Minsk,Супермаркеты
14.11.2026,-8.20,BYN,-8.20,BYN,Apteka Adel Minsk,Здоровье
10.11.2026,300.00,USD,1005.00,BYN,Perevod s valyutnogo scheta,Переводы
"""

def main():
    print(f"Генерация тестовых банковских выписок в: {OUTPUT_DIR}")

    # Запись Приорбанка в кодировке Windows-1251
    prior_path = OUTPUT_DIR / "priorbank_cp1251.csv"
    with open(prior_path, "w", encoding="cp1251", errors="replace") as f:
        f.write(PRIORBANK_CSV.strip())
    print(f" [✓] Приорбанк (CP1251): {prior_path.name}")

    # Запись Беларусбанка в кодировке Windows-1251
    belarus_path = OUTPUT_DIR / "belarusbank_cp1251.csv"
    with open(belarus_path, "w", encoding="cp1251", errors="replace") as f:
        f.write(BELARUSBANK_CSV.strip())
    print(f" [✓] Беларусбанк (CP1251): {belarus_path.name}")

    # Запись МТБанка в UTF-8
    mtbank_path = OUTPUT_DIR / "mtbank_utf8.csv"
    with open(mtbank_path, "w", encoding="utf-8") as f:
        f.write(MTBANK_CSV.strip())
    print(f" [✓] МТБанк (UTF-8): {mtbank_path.name}")

    # Запись Альфа-Банка в UTF-8
    alfa_path = OUTPUT_DIR / "alfabank_utf8.csv"
    with open(alfa_path, "w", encoding="utf-8") as f:
        f.write(ALFABANK_CSV.strip())
    print(f" [✓] Альфа-Банк (UTF-8): {alfa_path.name}")

    print("\nГотово! Файлы созданы для юнит-тестов парсеров ядра.")

if __name__ == "__main__":
    main()
