import itertools

def create_rotor_map(rotor_str):
    return {chr(i+65): rotor_str[i] for i in range(26)}

def create_reverse_rotor_map(rotor_str):
    return {rotor_str[i]: chr(i+65) for i in range(26)}

def enigma_decrypt(ciphertext, rotors, start_positions):
    pos = list(start_positions)
    plaintext = []
    
    for char in ciphertext:
        if char not in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
            plaintext.append(char)
            continue
            
        # Проход через роторы в обратном порядке
        c = char
        for i in range(2, -1, -1):
            rotor_map = rotors[i][1]  # reverse mapping
            c = rotor_map[c]
        
        plaintext.append(c)
        
        # Вращение роторов
        pos[0] = (pos[0] + 1) % 26
        if pos[0] == 0:
            pos[1] = (pos[1] + 1) % 26
            if pos[1] == 0:
                pos[2] = (pos[2] + 1) % 26
    
    return ''.join(plaintext)

# Загрузка роторов
with open('dict.txt', 'r') as f:
    rotor_strings = [line.strip() for line in f.readlines()]

# Создание прямых и обратных маппингов для каждого ротора
rotors_data = []
for rotor_str in rotor_strings:
    forward = create_rotor_map(rotor_str)
    reverse = create_reverse_rotor_map(rotor_str)
    rotors_data.append((forward, reverse))

ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG"

# Brute force
found = False
for rotor_indices in itertools.permutations(range(len(rotors_data)), 3):
    if found:
        break
        
    for pos1 in range(1, 11):
        if found:
            break
            
        for pos2 in range(1, 11):
            if found:
                break
                
            for pos3 in range(10, 26):
                selected_rotors = [rotors_data[rotor_indices[0]], 
                                 rotors_data[rotor_indices[1]], 
                                 rotors_data[rotor_indices[2]]]
                
                start_pos = [pos1, pos2, pos3]
                
                plaintext = enigma_decrypt(ciphertext, selected_rotors, start_pos)
                
                # Проверяем, содержит ли результат флаг
                if 'SIBINTEK' in plaintext:
                    print(f"Found! Rotors: {rotor_indices}, Positions: {start_pos}")
                    print(f"Plaintext: {plaintext}")
                    found = True
                    break

if not found:
    print("Solution not found. Trying different approach...")
