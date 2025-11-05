from Crypto.Util.number import inverse, bytes_to_long, long_to_bytes

p = 857504083339712752489993810777
q = 1029224947942998075080348647219
n = p * q
phi = (p - 1) * (q - 1)
e = 65537
d = inverse(e, phi)

print(f'Публичные ключи: {e, n}')
print(f'Приватные ключи: {d, n}')


message = b'HELLO_im_the_Flag'
ce = bytes_to_long(message)
c = pow(ce, e, n)
print(f'Шифротекст: {c}')

m = pow(c, d, n)
print(f'Дешифрация: {long_to_bytes(m)}')
