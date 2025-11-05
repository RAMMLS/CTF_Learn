import os 

def generate_key(length):
    return os.urandom(length)

def vernam_encrypt(plain_text, key):
    plain_text_bytes = plain_text.encode('utf-8')
    ciphertext = bytearray()
    for p, k in zip(plain_text_bytes, key):
        ciphertext.append(p^k)

    return bytes(ciphertext)

def vernam_decrypt(ciphertext, key):
    plain_text_bytes = bytearray()
    for c, k in zip(ciphertext, key):
        plain_text_bytes.append(c^k)

    return plain_text_bytes.decode('utf-8')


message = "Hello, World!"
key = generate_key(len(message))
ciphertext = vernam_encrypt(message, key)
print("Зашифрованное сообщение:", ciphertext)
decrypted_message = vernam_decrypt(ciphertext, key)
print("Расшифрованное сообщение:", decrypted_message)
