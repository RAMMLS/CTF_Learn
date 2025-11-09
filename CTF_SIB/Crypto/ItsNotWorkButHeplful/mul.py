import multiprocessing as mp
import numpy as np
from numba import jit

# Загрузка данных
with open('dict.txt', 'r') as f:
    rotor_strings = [line.strip() for line in f.readlines()]

rotors_num = []
for rotor_str in rotor_strings:
    rotor_arr = np.array([ord(c) - 65 for c in rotor_str], dtype=np.int32)
    rotors_num.append(rotor_arr)

ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG"
ciphertext_num = np.array([ord(c) - 65 for c in ciphertext], dtype=np.int32)

@jit(nopython=True)
def decrypt_and_check(ciphertext, rotor1, rotor2, rotor3, pos1, pos2, pos3):
    plaintext = np.empty_like(ciphertext)
    p1, p2, p3 = pos1, pos2, pos3
    
    for i in range(len(ciphertext)):
        char = ciphertext[i]
        
        char = (char + p3) % 26
        char = rotor3[char]
        char = (char - p3) % 26
        
        char = (char + p2) % 26
        char = rotor2[char]
        char = (char - p2) % 26
        
        char = (char + p1) % 26
        char = rotor1[char]
        char = (char - p1) % 26
        
        plaintext[i] = char
        
        p1 = (p1 + 1) % 26
        if p1 == 0:
            p2 = (p2 + 1) % 26
            if p2 == 0:
                p3 = (p3 + 1) % 26
    
    # Проверка на SIBINTEK
    for i in range(len(plaintext) - 7):
        if (plaintext[i] == 18 and plaintext[i+1] == 8 and plaintext[i+2] == 1 and
            plaintext[i+3] == 8 and plaintext[i+4] == 13 and plaintext[i+5] == 19 and
            plaintext[i+6] == 4 and plaintext[i+7] == 10):
            return True, plaintext
    
    return False, plaintext

def worker(rotor_indices_chunk, queue):
    for r1, r2, r3 in rotor_indices_chunk:
        rotor1 = rotors_num[r1]
        rotor2 = rotors_num[r2] 
        rotor3 = rotors_num[r3]
        
        for pos1 in range(1, 11):
            for pos2 in range(1, 11):
                for pos3 in range(10, 26):
                    found, plaintext = decrypt_and_check(
                        ciphertext_num, rotor1, rotor2, rotor3, pos1, pos2, pos3
                    )
                    if found:
                        plaintext_str = ''.join(chr(c + 65) for c in plaintext)
                        queue.put((r1, r2, r3, pos1, pos2, pos3, plaintext_str))
                        return

def main():
    # Генерируем все комбинации роторов
    total_rotors = len(rotors_num)
    rotor_combinations = []
    
    # Ограничим для теста
    test_range = min(50, total_rotors)
    for r1 in range(test_range):
        for r2 in range(test_range):
            if r2 == r1: continue
            for r3 in range(test_range):
                if r3 == r1 or r3 == r2: continue
                rotor_combinations.append((r1, r2, r3))
    
    # Разделяем на chunks для процессов
    num_processes = mp.cpu_count()
    chunk_size = len(rotor_combinations) // num_processes
    chunks = [rotor_combinations[i:i+chunk_size] for i in range(0, len(rotor_combinations), chunk_size)]
    
    queue = mp.Queue()
    processes = []
    
    print(f"Запуск {num_processes} процессов...")
    
    for chunk in chunks:
        p = mp.Process(target=worker, args=(chunk, queue))
        processes.append(p)
        p.start()
    
    # Ждем результат
    try:
        result = queue.get(timeout=3600)  # 1 час таймаут
        r1, r2, r3, pos1, pos2, pos3, plaintext = result
        
        print(f"УСПЕХ! Роторы: {r1},{r2},{r3}")
        print(f"Позиции: {pos1},{pos2},{pos3}")
        print(f"Расшифрованный текст: {plaintext}")
        
    except:
        print("Время вышло или результат не найден")
    
    # Завершаем процессы
    for p in processes:
        p.terminate()
        p.join()

if __name__ == "__main__":
    main()
