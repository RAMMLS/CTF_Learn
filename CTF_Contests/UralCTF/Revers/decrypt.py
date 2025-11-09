encrypted_data = [
    0x73, 0x24, 0x72, 0x16, 0x78, 0xC5, 0x75, 0x06,
    0x57, 0xA1, 0x48, 0x25, 0x77, 0x65, 0x44, 0x23,
    0x74, 0x1D, 0x08, 0xE3, 0x6C, 0x0B, 0x40, 0xCE,
    0x50, 0x4B, 0x67, 0x22, 0x15, 0x38, 0x6E
]

key = [0x38, 0x86, 0x19, 0x122, 0x59, 0x145, 0x51] 

def decrypt(data, key):
    result = []
    key_ptr = 0
    direction = 1
    key_start = 0
    key_end = len(key) - 1

    for i in range(0, len(data) * 2, 2):
        if i // 2 < len(data):
            decrypted_byte = data[i // 2] ^ key[key_ptr]
            result.append(decrypted_byte)

        if direction == 1:
            if key_ptr == key_end:
                direction = -1
                key_ptr -= 1
            else:
                key_ptr += 1
        else:
            if key_ptr == key_start:
                direction = 1
                key_ptr += 1
            else:
                key_ptr -= 1

    return result

flag = decrypt(encrypted_data, key)
print(flag)

