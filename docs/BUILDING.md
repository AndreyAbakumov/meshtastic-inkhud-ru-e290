# Сборка из исходников

База — официальный Meshtastic **2.7.26.54e0d8d**, commit `54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb`. Этот репозиторий содержит патч, каталог локализации и тесты. Полный снимок исходников с подмодулями находится также в релизе: `e290-inkhud-ru-source.tar.gz`.

Потребуются Git, Python 3.12, доступ к серверам зависимостей, C++17-компилятор Clang для host-тестов. Использованный toolchain и библиотеки перечислены в BUILD_INFO.md и файлах установочного комплекта.

```sh
git clone https://github.com/AndreyAbakumov/meshtastic-inkhud-ru-e290.git
cd meshtastic-inkhud-ru-e290
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements-build.txt
python scripts/prepare_firmware.py
export PLATFORMIO_CORE_DIR="$PWD/.platformio"
pio run -d firmware -e heltec-vision-master-e290-inkhud -t mtjson
python tests/run_lock.py
python tests/run_host.py
python tools/render_previews.py
```

Windows: активируйте `.venv\Scripts\activate.bat` в cmd или `.venv\Scripts\Activate.ps1` в PowerShell. Вместо `export` в PowerShell: `$env:PLATFORMIO_CORE_DIR = "$PWD/.platformio"`. Для host-тестов требуется доступная команда `clang++`; для одной только сборки прошивки она не нужна. На Linux/macOS команды выше выполняются без замены.

Артефакты находятся в `firmware/.pio/build/heltec-vision-master-e290-inkhud/`. Обычный `.bin` записывается в `0x10000`; `.factory.bin` — в `0x0`. Для чистой установки нужны также OTA и LittleFS — см. FLASHING.md.

Для изменения перевода редактируйте `localization/ru.json`, затем выполните `python tools/generate_localization.py` и пересоберите. Генератор выдаёт Windows-1251 для трёх штатных FreeSans; повторно декодировать UI-строки как UTF-8 нельзя. Изменения применяются только к E290.

Версия внутри устройства остаётся `2.7.26.54e0d8d`. Идентификатор релиза проекта `v1.1.0` не является версией протокола Meshtastic. Отсутствие Git-истории в полном архиве исходников может изменить автоматически вычисляемую строку версии; для штатной строки используйте закреплённый checkout плюс патч, как выше. Даты сборки и окружение могут менять хеш повторной сборки.

Тесты компилируют реальные методы и растеризатор, с заглушками сервисов устройства. Они не заменяют проверку радиосвязи и потребления на плате.
