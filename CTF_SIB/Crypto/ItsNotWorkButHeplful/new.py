import numpy as np
from numba import jit, prange
import itertools
from collections import Counter

# Загрузка роторов
with open('dict.txt', 'r') as f:
    rotor_strings = [line.strip() for line in f.readlines()]

# Преобразуем роторы в числовые массивы
rotors_num = []
for rotor_str in rotor_strings:
    rotor_arr = np.array([ord(c) - 65 for c in rotor_str], dtype=np.int32)
    rotors_num.append(rotor_arr)

ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG"
ciphertext_num = np.array([ord(c) - 65 for c in ciphertext], dtype=np.int32)

@jit(nopython=True)
def apply_20pos_switch(char, rotor, pos):
    # 20-позиционный переключатель
    char = (char + pos) % 20
    char = rotor[char] if char < len(rotor) else char
    char = (char - pos + 20) % 20
    return char

@jit(nopython=True)
def apply_6pos_switch(char, rotor, pos):
    # 6-позиционный переключатель
    char = (char + pos) % 6
    char = rotor[char] if char < len(rotor) else char
    char = (char - pos + 6) % 6
    return char

@jit(nopython=True)
def decrypt_with_feedback(ciphertext, rotor1, rotor2, rotor3, rotor6, pos1, pos2, pos3, pos6):
    plaintext = np.empty_like(ciphertext)
    p1, p2, p3, p6 = pos1, pos2, pos3, pos6
    
    for i in range(len(ciphertext)):
        char = ciphertext[i]
        
        # Входная коммутационная панель (упрощенная)
        # char остается тем же
        
        # Проход через 20-позиционные переключатели
        char = apply_20pos_switch(char, rotor1, p1)
        char = apply_20pos_switch(char, rotor2, p2)
        char = apply_20pos_switch(char, rotor3, p3)
        
        # Обратная связь от третьего переключателя
        feedback = apply_20pos_switch(char, rotor3, p3)
        # Упрощенная модель обратной связи
        char = (char + feedback) % 26
        
        # Проход через 6-позиционный переключатель
        char = apply_6pos_switch(char, rotor6, p6)
        
        # Выходная коммутационная панель (упрощенная)
        # char остается тем же
        
        plaintext[i] = char % 26
        
        # Вращение роторов
        p1 = (p1 + 1) % 20
        if p1 == 0:
            p2 = (p2 + 1) % 20
            if p2 == 0:
                p3 = (p3 + 1) % 20
                if p3 == 0:
                    p6 = (p6 + 1) % 6
    
    return plaintext

def create_6pos_rotors():
    """Создаем 6-позиционные роторы из 20-позиционных"""
    six_pos_rotors = []
    for rotor in rotors_num[:50]:  # Берем первые 50 роторов
        # Берем первые 6 позиций из 20-позиционного ротора
        six_rotor = rotor[:6].copy()
        six_pos_rotors.append(six_rotor)
    return six_pos_rotors

def optimized_brute_force():
    total_rotors = len(rotors_num)
    six_pos_rotors = create_6pos_rotors()
    
    print(f"Загружено {total_rotors} 20-позиционных роторов")
    print(f"Создано {len(six_pos_rotors)} 6-позиционных роторов")
    
    # Ограничиваем для тестирования
    rotor_limit = min(50, total_rotors)
    six_rotor_limit = min(20, len(six_pos_rotors))
    
    count = 0
    for r1 in range(rotor_limit):
        rotor1 = rotors_num[r1][:20]  # Берем первые 20 позиций
        for r2 in range(rotor_limit):
            if r2 == r1: continue
            rotor2 = rotors_num[r2][:20]
            for r3 in range(rotor_limit):
                if r3 == r1 or r3 == r2: continue
                rotor3 = rotors_num[r3][:20]
                
                for r6 in range(six_rotor_limit):
                    rotor6 = six_pos_rotors[r6]
                    
                    for pos1 in range(1, 11):  # 1-10
                        for pos2 in range(1, 11):  # 1-10  
                            for pos3 in range(10, 26):  # 10-25
                                for pos6 in range(1, 7):  # 1-6 для 6-позиционного
                                    count += 1
                                    if count % 100000 == 0:
                                        print(f"Проверено {count} комбинаций...")
                                    
                                    decrypted = decrypt_with_feedback(
                                        ciphertext_num, rotor1, rotor2, rotor3, rotor6, 
                                        pos1, pos2, pos3, pos6
                                    )
                                    
                                    decrypted_text = ''.join(chr(c + 65) for c in decrypted)
                                    
                                    # Проверяем на флаг
                                    if 'Sibintek{' in decrypted_text:
                                        print(f"НАЙДЕНО!")
                                        print(f"20-позиционные роторы: {r1},{r2},{r3}")
                                        print(f"6-позиционный ротор: {r6}")
                                        print(f"Позиции: {pos1},{pos2},{pos3},{pos6}")
                                        print(f"Текст: {decrypted_text}")
                                        
                                        # Извлекаем флаг
                                        start = decrypted_text.find('SIBINTEK{')
                                        end = decrypted_text.find('}', start)
                                        if start != -1 and end != -1:
                                            flag = decrypted_text[start:end+1]
                                            print(f"ФЛАГ: {flag}")
                                        return

if __name__ == "__main__":
    print("Запуск оптимизированного брутфорса...")
    optimized_brute_force()
