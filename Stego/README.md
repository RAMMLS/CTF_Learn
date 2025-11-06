```markdown
# Стеганография: Искусство скрытия информации

## Что такое стеганография?

**Стеганография** (от греч. στεγανός — скрытый и γράφω — пишу) — это наука и искусство скрытой передачи информации. В отличие от криптографии, которая делает сообщение нечитаемым для посторонних, стеганография скрывает сам факт существования сообщения.

**Ключевое отличие:**
*   **Криптография:** "Я не могу прочитать это сообщение."
*   **Стеганография:** "Я не вижу никакого сообщения."

## Основные методы

### Изображения
*   **LSB (Least Significant Bit):** Замена младших битов пикселей
*   **Методы на основе JPEG:** Манипуляции с DCT-коэффициентами
*   **Форматы:** PNG, BMP, JPEG, TIFF

### Аудиофайлы
*   **Методы:** LSB-кодирование, эхо-скрытие
*   **Форматы:** WAV, MP3, FLAC

### Другие контейнеры
*   Текст, сетевые пакеты, исполняемые файлы, музыкальные партитуры

## Инструменты для работы со стеганографией

### Командная строка

#### Steghide
```bash
# Установка
sudo apt install steghide

# Скрытие файла
steghide embed -cf image.jpg -ef secret.txt -p "password"

# Извлечение файла
steghide extract -sf image.jpg -p "password"

# Информация о файле
steghide info image.jpg
```

#### Binwalk
```bash
# Поиск встроенных файлов
binwalk image.jpg

# Извлечение файлов
binwalk -e image.jpg

# Подробное сканирование
binwalk -M image.jpg
```

#### Утилиты анализа
```bash
# Шестнадцатеричный дамп
xxd file.bin | head -20

# Поиск строк
strings -n 10 file.bin

# Проверка типа файла
file mystery.dat

# Метаданные
exiftool image.jpg
```

### Python библиотеки

#### Stegano
```python
from stegano import lsb

# Скрытие сообщения
secret = lsb.hide("image.png", "Secret message")
secret.save("output.png")

# Извлечение сообщения
message = lsb.reveal("output.png")
print(message)
```

#### Другие методы Stegano
```python
from stegano import exifHeader
from stegano import red

# Скрытие в EXIF
secret = exifHeader.hide("image.jpg", "output.jpg", "Secret")

# Скрытие в красном канале
secret = red.hide("image.png", "Secret message")
```

### Графические инструменты

- **Stegsolve** - визуальный анализ цветовых плоскостей
- **OpenStego** - кроссплатформенное GUI-приложение

### Онлайн-инструменты

- **AperiSolve** - автоматический анализ изображений: https://www.aperisolve.com/
- **PlanetCalc LSB Decoder** - декодирование LSB: https://planetcalc.com/9345/
- **dCode Music Cipher** - шифры в музыкальных нотах: https://www.dcode.fr/music-sheet-cipher

## Специальные виды стеганографии

### Музыкальная стеганография
Использование нотных партитур для скрытия сообщений через:
- Последовательности нот
- Длительности нот
- Музыкальные символы
- Позиции на нотном стане

**Инструменты:** dCode Music Sheet Cipher

### Текстовая стеганография
- Невидимые символы (Zero-Width Characters)
- Изменение форматирования
- Специальные юникод-символы

## Типичный рабочий процесс анализа

1. **Базовые проверки:**
```bash
file suspicious.jpg
exiftool suspicious.jpg
strings suspicious.jpg
```

2. **Автоматический анализ:**
- Загрузка на AperiSolve
- Проверка в PlanetCalc LSB Decoder

3. **Инструментальный анализ:**
```bash
binwalk -e suspicious.jpg
steghide extract -sf suspicious.jpg -p ""
```

4. **Программный анализ:**
```python
from stegano import lsb
try:
    print(lsb.reveal("suspicious.png"))
except:
    print("Не найдено")
```

5. **Специализированный анализ:**
- Для музыкальных файлов: dCode Music Cipher
- Для текстов: анализ юникод-символов

6. **Визуальный анализ:**
- Использование Stegsolve для просмотра цветовых плоскостей

7. **Ручной анализ:**
```bash
xxd suspicious.bin | grep -A 5 -B 5 "PK"
```

## Заключение

Стеганография требует комплексного подхода. Начинайте с простых проверок, переходите к автоматическим инструментам и завершайте ручным анализом при необходимости. Сочетание консольных утилит, Python-скриптов и онлайн-сервисов дает наилучшие результаты для обнаружения скрытых данных в различных форматах - от изображений до музыкальных партитур.
```
