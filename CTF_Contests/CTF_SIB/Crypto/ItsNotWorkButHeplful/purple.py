import itertools
import string
from collections import Counter

# Загрузка роторов
with open('dict.txt', 'r') as f:
    rotors = [line.strip().upper() for line in f.readlines() if len(line.strip()) == 26]

print(f"Загружено {len(rotors)} роторов")

ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG"

# Purple: разделение на гласные и согласные
VOWELS = "AEIOUY"  # 6 букв
CONSONANTS = "BCDFGHJKLMNPQRSTVWXZ"  # 20 букв

def purple_decrypt(consonant_rotors, vowel_rotor, c_pos1, c_pos2, c_pos3, v_pos, text):
    """Упрощенная модель дешифрования Purple"""
    result = []
    
    # Начальные позиции
    cp1, cp2, cp3 = c_pos1, c_pos2, c_pos3
    vp = v_pos
    
    for char in text:
        if char not in string.ascii_uppercase:
            result.append(char)
            continue
            
        char_num = ord(char) - 65
        
        if char in CONSONANTS:
            # Обработка согласных через 20-позиционные переключатели
            idx = CONSONANTS.index(char)
            
            # Проход через 3 переключателя для согласных
            idx = (idx + cp3) % 20
            idx = ord(consonant_rotors[2][idx]) - 65
            idx = (idx - cp3 + 20) % 20
            
            idx = (idx + cp2) % 20
            idx = ord(consonant_rotors[1][idx]) - 65
            idx = (idx - cp2 + 20) % 20
            
            idx = (idx + cp1) % 20
            idx = ord(consonant_rotors[0][idx]) - 65
            idx = (idx - cp1 + 20) % 20
            
            result.append(CONSONANTS[idx])
            
            # Вращение только для согласных переключателей
            cp1 = (cp1 + 1) % 20
            if cp1 == 0:
                cp2 = (cp2 + 1) % 20
                if cp2 == 0:
                    cp3 = (cp3 + 1) % 20
            
        else:  # Гласные
            # Обработка гласных через 6-позиционный переключатель
            idx = VOWELS.index(char)
            
            idx = (idx + vp) % 6
            idx = ord(vowel_rotor[idx]) - 65
            idx = (idx - vp + 6) % 6
            
            result.append(VOWELS[idx])
            
            # Вращение для гласного переключателя
            vp = (vp + 1) % 6
    
    return ''.join(result)

def analyze_purple_patterns(text):
    """Анализ текста на соответствие Purple-шаблонам"""
    if len(text) < 50:
        return False
        
    # В Purple часто чередуются гласные/согласные
    vowel_count = sum(1 for c in text if c in VOWELS)
    consonant_count = sum(1 for c in text if c in CONSONANTS)
    
    # Проверяем разумное соотношение (~40% гласных в английском)
    vowel_ratio = vowel_count / (vowel_count + consonant_count)
    
    return 0.35 <= vowel_ratio <= 0.45

# Purple-ориентированный брутфорс
def purple_bruteforce():
    TEST_ROTORS = min(50, len(rotors))
    print(f"Purple брутфорс: тестируем {TEST_ROTORS} роторов")
    
    found = False
    count = 0
    
    # Перебираем комбинации для согласных (3 ротора) и гласных (1 ротор)
    for consonant_combo in itertools.permutations(range(TEST_ROTORS), 3):
        if found:
            break
            
        consonant_rotors = [rotors[consonant_combo[0]], 
                          rotors[consonant_combo[1]], 
                          rotors[consonant_combo[2]]]
        
        for vowel_rotor_idx in range(TEST_ROTORS):
            if found:
                break
                
            vowel_rotor = rotors[vowel_rotor_idx]
            
            # Позиции согласно hint.txt + позиция для гласного ротора
            for c_pos1 in range(1, 11):
                for c_pos2 in range(1, 11):
                    for c_pos3 in range(10, 26):
                        for v_pos in range(1, 7):  # 1-6 для 6-позиционного
                            count += 1
                            if count % 10000 == 0:
                                print(f"Проверено {count} Purple комбинаций...")
                            
                            plaintext = purple_decrypt(
                                consonant_rotors, vowel_rotor,
                                c_pos1, c_pos2, c_pos3, v_pos,
                                ciphertext
                            )
                            
                            # Проверка на флаг
                            if 'SIBINTEK{' in plaintext:
                                print(f"\n=== PURPLE НАЙДЕНО! ===")
                                print(f"Согласные роторы: {consonant_combo}")
                                print(f"Гласный ротор: {vowel_rotor_idx}")
                                print(f"Позиции согласных: {c_pos1}, {c_pos2}, {c_pos3}")
                                print(f"Позиция гласных: {v_pos}")
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
        print("Purple флаг не найден в тестовом диапазоне")

# Сначала попробуем Purple
print("Запуск Purple-ориентированного брутфорса...")
purple_bruteforce()
