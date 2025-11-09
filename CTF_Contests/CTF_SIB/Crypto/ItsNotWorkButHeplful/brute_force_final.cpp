#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <cstring>

using namespace std;

atomic<bool> found(false);
mutex cout_mutex;

vector<vector<int>> rotors;
vector<int> ciphertext_num;

// Функция для преобразования строки в числовой массив
vector<int> stringToNum(const string& str) {
    vector<int> result;
    for (char c : str) {
        if (c >= 'A' && c <= 'Z') {
            result.push_back(c - 'A');
        }
    }
    return result;
}

// Функция для преобразования числового массива в строку
string numToString(const vector<int>& nums) {
    string result;
    for (int num : nums) {
        result += char(num + 'A');
    }
    return result;
}

// Оптимизированная функция дешифрования
string decrypt(const vector<int>& rotor1, const vector<int>& rotor2, const vector<int>& rotor3, 
               int pos1, int pos2, int pos3) {
    vector<int> plaintext(ciphertext_num.size());
    int p1 = pos1, p2 = pos2, p3 = pos3;
    int n = ciphertext_num.size();
    
    for (int i = 0; i < n; i++) {
        int c = ciphertext_num[i];
        
        // Обратный проход через роторы
        c = (c + p3) % 26;
        c = rotor3[c];
        c = (c - p3 + 26) % 26;
        
        c = (c + p2) % 26;
        c = rotor2[c];
        c = (c - p2 + 26) % 26;
        
        c = (c + p1) % 26;
        c = rotor1[c];
        c = (c - p1 + 26) % 26;
        
        plaintext[i] = c;
        
        // Вращение роторов
        p1 = (p1 + 1) % 26;
        if (p1 == 0) {
            p2 = (p2 + 1) % 26;
            if (p2 == 0) {
                p3 = (p3 + 1) % 26;
            }
        }
    }
    
    return numToString(plaintext);
}

// Проверка на наличие флага
bool containsFlag(const string& text) {
    return text.find("SIBINTEK{") != string::npos;
}

void worker(int start_r1, int end_r1, int total_rotors) {
    for (int r1 = start_r1; r1 < end_r1 && !found; r1++) {
        if (r1 % 100 == 0) {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Проверка ротора " << r1 << " из " << total_rotors << endl;
        }
        
        for (int r2 = 0; r2 < total_rotors && !found; r2++) {
            if (r2 == r1) continue;
            for (int r3 = 0; r3 < total_rotors && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                
                for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                    for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                        for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                            string plaintext = decrypt(rotors[r1], rotors[r2], rotors[r3], pos1, pos2, pos3);
                            
                            if (containsFlag(plaintext)) {
                                found = true;
                                lock_guard<mutex> lock(cout_mutex);
                                cout << "\n=== НАЙДЕН ФЛАГ! ===" << endl;
                                cout << "Роторы: " << r1 << ", " << r2 << ", " << r3 << endl;
                                cout << "Стартовые позиции: " << pos1 << ", " << pos2 << ", " << pos3 << endl;
                                cout << "Полный текст: " << plaintext << endl;
                                
                                // Извлекаем флаг
                                size_t start = plaintext.find("SIBINTEK{");
                                size_t end = plaintext.find('}', start);
                                if (start != string::npos && end != string::npos) {
                                    string flag = plaintext.substr(start, end - start + 1);
                                    cout << "ФЛАГ: " << flag << endl;
                                }
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Функция для загрузки роторов с улучшенной обработкой
bool loadRotors(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Ошибка: Не могу открыть файл " << filename << endl;
        cout << "Пробую альтернативные имена файлов..." << endl;
        
        // Попробуем другие возможные имена файлов
        vector<string> possible_names = {
            "dict.txt", "dict (1).txt", "dict(1).txt", 
            "./dict.txt", "./dict (1).txt", "./dict(1).txt"
        };
        
        for (const auto& name : possible_names) {
            file.open(name);
            if (file.is_open()) {
                cout << "Успешно открыт файл: " << name << endl;
                break;
            }
        }
        
        if (!file.is_open()) {
            cout << "Не удалось открыть файл с роторами." << endl;
            cout << "Убедитесь, что файл находится в той же директории, что и программа." << endl;
            return false;
        }
    }

    string line;
    cout << "Загрузка роторов..." << endl;
    int line_count = 0;
    int valid_rotors = 0;
    
    while (getline(file, line)) {
        line_count++;
        
        // Удаляем пробелы и непечатаемые символы
        line.erase(remove_if(line.begin(), line.end(), [](char c) {
            return !isalpha(c);
        }), line.end());
        
        // Преобразуем в верхний регистр
        transform(line.begin(), line.end(), line.begin(), ::toupper);
        
        if (line.length() == 26) {
            vector<int> rotor(26);
            bool valid = true;
            
            for (int i = 0; i < 26; i++) {
                if (line[i] < 'A' || line[i] > 'Z') {
                    valid = false;
                    break;
                }
                rotor[i] = line[i] - 'A';
            }
            
            if (valid) {
                rotors.push_back(rotor);
                valid_rotors++;
            }
        }
    }
    
    file.close();
    
    cout << "Прочитано строк: " << line_count << endl;
    cout << "Загружено валидных роторов: " << valid_rotors << endl;
    
    if (valid_rotors == 0) {
        cout << "Не найдено ни одного валидного ротора!" << endl;
        cout << "Проверьте формат файла. Каждый ротор должен быть строкой из 26 букв." << endl;
        return false;
    }
    
    return true;
}

int main() {
    // Загрузка роторов
    if (!loadRotors("dict (1).txt")) {
        return 1;
    }
    
    // Подготовка шифртекста
    string ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG";
    ciphertext_num = stringToNum(ciphertext);
    
    int total_rotors = rotors.size();
    int num_threads = thread::hardware_concurrency();
    
    cout << "\nНастройки брутфорса:" << endl;
    cout << "Количество роторов: " << total_rotors << endl;
    cout << "Количество потоков: " << num_threads << endl;
    
    // Расчет общего количества комбинаций
    long long total_combinations = (long long)total_rotors * 
                                  (total_rotors - 1) * 
                                  (total_rotors - 2) * 
                                  10 * 10 * 16;
    
    cout << "Общее количество комбинаций: " << total_combinations << endl;
    cout << "Запуск брутфорса..." << endl;
    
    // Распределяем работу по потокам
    vector<thread> threads;
    int rotors_per_thread = total_rotors / num_threads;
    
    auto start_time = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; i++) {
        int start = i * rotors_per_thread;
        int end = (i == num_threads - 1) ? total_rotors : (i + 1) * rotors_per_thread;
        threads.emplace_back(worker, start, end, total_rotors);
    }
    
    // Ждем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    
    cout << "\nВремя выполнения: " << duration.count() << " секунд" << endl;
    
    if (!found) {
        cout << "Флаг не найден." << endl;
        cout << "Возможные причины:" << endl;
        cout << "1. Флаг имеет другой формат" << endl;
        cout << "2. Алгоритм шифрования отличается от предполагаемого" << endl;
        cout << "3. Диапазоны стартовых позиций неверны" << endl;
    }
    
    return 0;
}
