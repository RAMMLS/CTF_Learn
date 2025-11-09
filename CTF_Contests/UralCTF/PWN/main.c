#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

int main() {
    time_t v10;
    uint32_t seed;
    int64_t v5, v11;

    // Получаем текущее время
    v10 = time(NULL);

    // Вычисляем seed, как в оригинальном коде
    seed = (((((uint32_t)v10 << 13) ^ (uint32_t)v10) >> 7) ^ ((uint32_t)v10 << 13) ^ v10) + 305419896;

    // Инициализируем генератор случайных чисел
    srand(((((4919 * seed) ^ seed) >> 3) ^ (-559038737 * ((4919 * seed) ^ seed))) + 4919);

    // Генерируем 64-битное число
    v5 = (int64_t)rand() << 32;
    v11 = v5 | rand();

    // Выводим число в шестнадцатеричном формате
    printf("%llx\n", v11);

    return 0;
}
