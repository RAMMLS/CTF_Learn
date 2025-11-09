import itertools
from collections import Counter

# Загрузка роторов
with open('dict.txt', 'r') as f:
    rotors = [line.strip().upper() for line in f.readlines() if len(line.strip()) == 26]

print(f"Загружено {len(rotors)} роторов")

ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG"

def decrypt_enigma(rotor1, rotor2, rotor3, pos1, pos2, pos3, text):
    """Упрощенная дешифровка Энигмы с 3 роторами"""
    result = []
    p1, p2, p3 = pos1, pos2, pos3
    
    for char in text:
        if not char.isalpha():
            result.append(char)
            continue
            
        # Преобразуем букву в число (0-25)
        c = ord(char) - 65
        
        # Обратный проход через роторы
        c = (c + p3) % 26
        c = ord(rotor3[c]) - 65
        c = (c - p3 + 26) % 26
        
        c = (c + p2) % 26
        c = ord(rotor2[c]) - 65
        c = (c - p2 + 26) % 26
        
        c = (c + p1) % 26
        c = ord(rotor1[c]) - 65
        c = (c - p1 + 26) % 26
        
        result.append(chr(c + 65))
        
        # Вращение роторов
        p1 = (p1 + 1) % 26
        if p1 == 0:
            p2 = (p2 + 1) % 26
            if p2 == 0:
                p3 = (p3 + 1) % 26
    
    return ''.join(result)

def is_english_text(text):
    """Проверка на английский текст"""
    text = text.upper()
    
    # Проверяем наличие флага
    if 'SIBINTEK{' in text:
        return True
        
    # Проверяем частые английские биграммы
    common_bigrams = ['TH', 'HE', 'IN', 'ER', 'AN', 'RE', 'ON', 'AT']
    bigrams = [text[i:i+2] for i in range(len(text)-1)]
    bigram_count = sum(1 for bg in common_bigrams if bg in text)
    
    return bigram_count >= 3

# Ограничиваем количество роторов для теста
TEST_ROTORS = min(100, len(rotors))
print(f"Тестируем первые {TEST_ROTORS} роторов")

found = False
for i, (r1, r2, r3) in enumerate(itertools.permutations(range(TEST_ROTORS), 3)):
    if found:
        break
        
    if i % 1000 == 0:
        print(f"Проверено {i} комбинаций роторов...")
    
    for pos1 in range(1, 11):      # 1-10
        for pos2 in range(1, 11):  # 1-10  
            for pos3 in range(10, 26):  # 10-25
                plaintext = decrypt_enigma(rotors[r1], rotors[r2], rotors[r3], 
                                         pos1, pos2, pos3, ciphertext)
                
                if 'SIBINTEK{' in plaintext:
                    print(f"\n=== НАЙДЕНО! ===")
                    print(f"Роторы: {r1}, {r2}, {r3}")
                    print(f"Позиции: {pos1}, {pos2}, {pos3}")
                    print(f"Текст: {plaintext}")
                    
                    # Извлекаем флаг
                    start = plaintext.find('SIBINTEK{')
                    end = plaintext.find('}', start)
                    if start != -1 and end != -1:
                        flag = plaintext[start:end+1]
                        print(f"ФЛАГ: {flag}")
                    
                    found = True
                    break

if not found:
    print("Флаг не найден в тестовом диапазоне. Увеличьте TEST_ROTORS.")
