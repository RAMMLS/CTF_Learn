import base64
import re

# Читаем дамп памяти notepad
with open('pid.4972.dmp', 'rb') as f:
    data = f.read()

# Ищем строки, похожие на Base64
text = data.decode('latin-1')
base64_pattern = re.compile(r'[A-Za-z0-9+/]{20,}={0,2}')

for match in base64_pattern.findall(text):
    try:
        decoded = base64.b64decode(match).decode('utf-8', errors='ignore')
        if 'Sibintek{' in decoded:
            print(f"Found: {decoded}")
            print(f"Base64: {match}")
            break
    except:
        continue
