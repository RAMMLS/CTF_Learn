import string

# Letter to index and index to letter
def letter_to_num(c):
    return ord(c) - ord('A')

def num_to_letter(n):
    return chr(n + ord('A'))

# Read dict.txt
with open('dict.txt', 'r') as f:
    lines = f.readlines()
# Remove newlines and take only lines with 26 letters
rotors = [line.strip() for line in lines if len(line.strip()) == 26]

# Take the first three rotors
R1 = rotors[0]
R2 = rotors[1]
R3 = rotors[2]

# Precompute the permutations as lists of integers
R1_list = [letter_to_num(c) for c in R1]
R2_list = [letter_to_num(c) for c in R2]
R3_list = [letter_to_num(c) for c in R3]

# Compute inverse permutations
def inverse_permutation(perm):
    inv = [0] * len(perm)
    for i, p in enumerate(perm):
        inv[p] = i
    return inv

R1_inv = inverse_permutation(R1_list)
R2_inv = inverse_permutation(R2_list)
R3_inv = inverse_permutation(R3_list)

# Ciphertext
ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG"

cipher_num = [letter_to_num(c) for c in ciphertext]

# Try initial positions
for p1 in range(0,10):   # first rotor initial position
    for p2 in range(0,10): # second rotor initial position
        for p3 in range(9,25): # third rotor initial position
            positions = [p1, p2, p3]
            plaintext = []
            # Make copies of current positions
            current_pos = positions.copy()
            for c in cipher_num:
                # Rotate rotors
                current_pos[0] = (current_pos[0] + 1) % 26
                if current_pos[0] == 0:
                    current_pos[1] = (current_pos[1] + 1) % 26
                    if current_pos[1] == 0:
                        current_pos[2] = (current_pos[2] + 1) % 26

                # Decrypt one letter
                x = c
                # Go through R3_inv, then R2_inv, then R1_inv
                x = ( R3_inv[(x + current_pos[2]) % 26] - current_pos[2] ) % 26
                x = ( R2_inv[(x + current_pos[1]) % 26] - current_pos[1] ) % 26
                x = ( R1_inv[(x + current_pos[0]) % 26] - current_pos[0] ) % 26
                plaintext.append(num_to_letter(x))

            plain_str = ''.join(plaintext)
            # Check if plain_str contains "THE" and "SIBINTEK"
            if "THE" in plain_str and "SIBINTEK" in plain_str:
                print(f"Found with p1={p1}, p2={p2}, p3={p3}")
                print(plain_str)
                exit(0)
