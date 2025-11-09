import base64
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad


import re

# Ваша сырая строка, извлеченная из бинарника
raw_data = "ваша_строка_с_мусором_здесь"

# Удаляем все символы, кроме допустимых в Base64
cleaned_b64 = re.sub(r'[^A-Za-z0-9+/=]', '', raw_data)

print("Очищенная строка Base64:", cleaned_b64)

# 1. Вставьте СЮДА вашу исходную строку Base64, которую вы декодировали
encrypted_b64_data = "ВАША_СТРОКА_BASE64_ЗДЕСЬ=="

# 2. Ключ из дизассемблированного кода (SHA-256 хэш)
hex_key = "D3A2E9642D1327532E5831C08F0F0F1D7667F17B056055A5A60F14C7983D615B"

# 3. Преобразуем ключ из HEX в байты
key = bytes.fromhex(hex_key)

# 4. Декодируем данные из Base64
encrypted_data = base64.b64decode(encrypted_b64_data)

# 5. Извлекаем IV (первые 16 байт) и сам шифртекст (все остальное)
iv = encrypted_data[:16]
ciphertext = encrypted_data[16:]

# 6. Создаем дешифратор AES в режиме CBC
cipher = AES.new(key, AES.MODE_CBC, iv=iv)

# 7. Дешифруем и убираем padding
decrypted_data = unpad(cipher.decrypt(ciphertext), AES.block_size)

# 8. Выводим результат в виде читаемой строки
print(decrypted_data.decode('utf-8'))

