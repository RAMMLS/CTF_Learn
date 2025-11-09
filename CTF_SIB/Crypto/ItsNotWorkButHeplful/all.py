import numpy as np
from numba import jit, prange
import itertools

# Загрузка роторов
with open('dict.txt', 'r') as f:
    rotor_strings = [line.strip() for line in f.readlines()]

# Преобразуем роторы в числовые массивы для скорости
rotors_num = []
for rotor_str in rotor_strings:
    rotor_arr = np.array([ord(c) - 65 for c in rotor_str], dtype=np.int32)
    rotors_num.append(rotor_arr)

ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG"
ciphertext_num = np.array([ord(c) - 65 for c in ciphertext], dtype=np.int32)

@jit(nopython=True)
def apply_rotor(char, rotor, pos):
    char = (char + pos) % 26
    char = rotor[char]
    char = (char - pos) % 26
    return char

@jit(nopython=True)
def decrypt_with_rotors(ciphertext, rotor1, rotor2, rotor3, pos1, pos2, pos3):
    plaintext = np.empty_like(ciphertext)
    p1, p2, p3 = pos1, pos2, pos3
    
    for i in range(len(ciphertext)):
        char = ciphertext[i]
        
        # Обратный проход через роторы
        char = apply_rotor(char, rotor3, p3)
        char = apply_rotor(char, rotor2, p2)
        char = apply_rotor(char, rotor1, p1)
        
        plaintext[i] = char
        
        # Вращение роторов
        p1 = (p1 + 1) % 26
        if p1 == 0:
            p2 = (p2 + 1) % 26
            if p2 == 0:
                p3 = (p3 + 1) % 26
    
    return plaintext

@jit(nopython=True)
def check_english_text(text_arr, threshold=0.7):
    # Проверяем, содержит ли текст английские слова
    common_words = [
        'THE', 'AND', 'ING', 'HER', 'HAT', 'HIS', 'THA', 'ERE', 'FOR', 'ENT',
        'ION', 'TER', 'WAS', 'YOU', 'ITH', 'VER', 'ALL', 'WIT', 'THI', 'TIO'
    ]
    
    text = ''.join(chr(c + 65) for c in text_arr[:50])  # Первые 50 символов
    text = text.upper()
    
    found_count = 0
    for word in common_words:
        if word in text:
            found_count += 1
    
    return found_count >= len(common_words) * threshold

def brute_force_parallel():
    total_rotors = len(rotors_num)
    
    # Ограничим для теста, потом можно убрать
    rotor_range = min(100, total_rotors)  # Берем первые 100 роторов для теста
    
    for r1 in prange(rotor_range):
        rotor1 = rotors_num[r1]
        for r2 in prange(total_rotors):
            if r2 == r1: continue
            rotor2 = rotors_num[r2]
            for r3 in prange(total_rotors):
                if r3 == r1 or r3 == r2: continue
                rotor3 = rotors_num[r3]
                
                for pos1 in range(1, 11):
                    for pos2 in range(1, 11):
                        for pos3 in range(10, 26):
                            decrypted = decrypt_with_rotors(
                                ciphertext_num, rotor1, rotor2, rotor3, pos1, pos2, pos3
                            )
                            
                            # Проверяем на наличие флага
                            decrypted_text = ''.join(chr(c + 65) for c in decrypted)
                            if 'SIBINTEK' in decrypted_text:
                                print(f"НАЙДЕНО! Роторы: {r1},{r2},{r3} Позиции: {pos1},{pos2},{pos3}")
                                print(f"Текст: {decrypted_text}")
                                return
                            
                            # Дополнительная проверка на английский текст
                            if check_english_text(decrypted):
                                print(f"Возможный вариант: Роторы: {r1},{r2},{r3} Позиции: {pos1},{pos2},{pos3}")
                                print(f"Текст: {decrypted_text}")

if __name__ == "__main__":
    print("Запуск брутфорса...")
    brute_force_parallel()
