#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <map>
#include <cmath>

using namespace std;

atomic<bool> found(false);
atomic<long long> total_processed(0);
mutex cout_mutex;

vector<vector<int>> rotors;
vector<int> ciphertext_num;

// Частоты букв в английском языке
const vector<double> english_freq = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015, // A-G
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749, // H-N
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758, // O-U
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074                   // V-Z
};

// Расширенный список английских слов для проверки
const vector<string> english_words = {
    "THE", "AND", "ING", "HER", "HAT", "HIS", "THA", "ERE", "FOR", "ENT",
    "ION", "TER", "WAS", "YOU", "ITH", "VER", "ALL", "WIT", "THI", "TIO",
    "WITH", "HAVE", "THIS", "THAT", "FROM", "THEY", "BEEN", "WILL", "WOULD",
    "THERE", "THEIR", "WHICH", "COULD", "OTHER", "ABOUT", "INTO", "THAN",
    "ITS", "TIME", "ONLY", "LIKE", "THEN", "SOME", "THEM", "WHEN", "MAKE",
    "MORE", "VERY", "JUST", "YOUR", "SHOULD", "PEOPLE", "ALSO", "THESE"
};

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

// Улучшенная проверка на английский текст
bool isLikelyEnglish(const string& text, double threshold = 0.6) {
    if (text.length() < 20) return false;
    
    // Проверка частот букв (хи-квадрат)
    vector<int> counts(26, 0);
    int total_letters = 0;
    
    for (char c : text) {
        if (c >= 'A' && c <= 'Z') {
            counts[c - 'A']++;
            total_letters++;
        }
    }
    
    if (total_letters < 10) return false;
    
    double chi_squared = 0.0;
    for (int i = 0; i < 26; i++) {
        double expected = english_freq[i] * total_letters;
        double observed = counts[i];
        if (expected > 0) {
            chi_squared += pow(observed - expected, 2) / expected;
        }
    }
    
    // В английском тексте хи-квадрат обычно меньше 150
    if (chi_squared > 150) return false;
    
    // Проверка наличия английских слов
    string upper_text = text;
    transform(upper_text.begin(), upper_text.end(), upper_text.begin(), ::toupper);
    
    int word_matches = 0;
    for (const string& word : english_words) {
        if (upper_text.find(word) != string::npos) {
            word_matches++;
        }
    }
    
    double match_ratio = (double)word_matches / english_words.size();
    return match_ratio >= threshold;
}

// Двунаправленный воркер - обрабатывает с начала и с конца
void bidirectional_worker(int thread_id, int start_r1, int end_r1, int total_rotors, long long total_combinations, bool reverse) {
    auto thread_start = chrono::high_resolution_clock::now();
    long long local_processed = 0;
    long long last_reported = 0;
    
    // Определяем направление обхода
    int r1_start, r1_end, r1_step;
    if (reverse) {
        r1_start = end_r1 - 1;
        r1_end = start_r1 - 1;
        r1_step = -1;
    } else {
        r1_start = start_r1;
        r1_end = end_r1;
        r1_step = 1;
    }
    
    for (int r1 = r1_start; (reverse && r1 > r1_end) || (!reverse && r1 < r1_end); r1 += r1_step) {
        if (found) break;
        
        // Логирование прогресса
        if (local_processed % 10000 == 0) {
            auto now = chrono::high_resolution_clock::now();
            auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(now - thread_start);
            long elapsed = elapsed_seconds.count();
            
            lock_guard<mutex> lock(cout_mutex);
            cout << "[Поток " << thread_id << (reverse ? "R" : "F") << "] Ротор: " << r1 
                 << ", комбинаций: " << local_processed 
                 << ", скорость: " << (elapsed > 0 ? local_processed / elapsed : local_processed) << " комб/с" << endl;
        }
        
        for (int r2 = 0; r2 < total_rotors && !found; r2++) {
            if (r2 == r1) continue;
            
            for (int r3 = 0; r3 < total_rotors && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                
                for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                    for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                        for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                            string plaintext = decrypt(rotors[r1], rotors[r2], rotors[r3], pos1, pos2, pos3);
                            local_processed++;
                            total_processed++;
                            
                            if (containsFlag(plaintext)) {
                                found = true;
                                auto end_time = chrono::high_resolution_clock::now();
                                auto total_elapsed = chrono::duration_cast<chrono::seconds>(end_time - thread_start);
                                
                                lock_guard<mutex> lock(cout_mutex);
                                cout << "\n=== НАЙДЕН ФЛАГ! ===" << endl;
                                cout << "Поток: " << thread_id << (reverse ? "R" : "F") << endl;
                                cout << "Роторы: " << r1 << ", " << r2 << ", " << r3 << endl;
                                cout << "Стартовые позиции: " << pos1 << ", " << pos2 << ", " << pos3 << endl;
                                cout << "Обработано комбинаций: " << local_processed << endl;
                                cout << "Время работы: " << total_elapsed.count() << " секунд" << endl;
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
                            
                            // Дополнительная проверка на английский текст
                            if (isLikelyEnglish(plaintext)) {
                                lock_guard<mutex> lock(cout_mutex);
                                cout << "[ВОЗМОЖНЫЙ ТЕКСТ] Поток " << thread_id << (reverse ? "R" : "F") 
                                     << ", роторы: " << r1 << "," << r2 << "," << r3 
                                     << ", позиции: " << pos1 << "," << pos2 << "," << pos3 << endl;
                                cout << "Текст: " << plaintext.substr(0, 100) << "..." << endl;
                            }
                        }
                    }
                }
            }
        }
    }
    
    auto thread_end = chrono::high_resolution_clock::now();
    auto thread_elapsed = chrono::duration_cast<chrono::seconds>(thread_end - thread_start);
    
    lock_guard<mutex> lock(cout_mutex);
    cout << "[Поток " << thread_id << (reverse ? "R" : "F") << "] ЗАВЕРШЕН. Обработано: " << local_processed 
         << " комбинаций, время: " << thread_elapsed.count() << "с" << endl;
}

// Функция для загрузки роторов с улучшенной обработкой
bool loadRotors(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Ошибка: Не могу открыть файл " << filename << endl;
        
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
        return false;
    }
    
    return true;
}

int main() {
    auto program_start = chrono::high_resolution_clock::now();
    
    // Загрузка роторов
    if (!loadRotors("dict.txt")) {
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
    cout << "Запуск двунаправленного брутфорса..." << endl;
    
    // Распределяем работу по потокам (прямое и обратное направление)
    vector<thread> threads;
    int rotors_per_thread = total_rotors / (num_threads * 2);
    
    auto start_time = chrono::high_resolution_clock::now();
    
    // Запускаем потоки в прямом и обратном направлении
    for (int i = 0; i < num_threads; i++) {
        int start = i * rotors_per_thread;
        int end = (i == num_threads - 1) ? total_rotors / 2 : (i + 1) * rotors_per_thread;
        
        // Прямой поток
        cout << "Запуск прямого потока " << i << " (роторы " << start << " - " << end-1 << ")" << endl;
        threads.emplace_back(bidirectional_worker, i, start, end, total_rotors, total_combinations, false);
        
        // Обратный поток
        int rev_start = total_rotors - 1 - start;
        int rev_end = total_rotors - 1 - end;
        cout << "Запуск обратного потока " << i << " (роторы " << rev_start << " - " << rev_end+1 << ")" << endl;
        threads.emplace_back(bidirectional_worker, i, rev_end, rev_start, total_rotors, total_combinations, true);
    }
    
    // Поток для отображения общего прогресса
    thread progress_thread([total_combinations, program_start]() {
        while (!found) {
            this_thread::sleep_for(chrono::seconds(10));
            
            auto now = chrono::high_resolution_clock::now();
            auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(now - program_start);
            long long elapsed = elapsed_seconds.count();
            long long processed = total_processed;
            double progress = 100.0 * processed / total_combinations;
            
            long long speed = (elapsed > 0) ? (processed / elapsed) : processed;
            
            lock_guard<mutex> lock(cout_mutex);
            cout << "\n[ОБЩИЙ ПРОГРЕСС] " << fixed << setprecision(4) << progress 
                 << "%, обработано: " << processed << "/" << total_combinations
                 << ", время: " << elapsed << "с" 
                 << ", скорость: " << speed << " комб/с" << endl;
                 
            // Примерное время до завершения
            if (progress > 0) {
                long long remaining_time = (100.0 - progress) * elapsed / progress;
                cout << "[ОЦЕНКА] Примерное время до завершения: " << remaining_time << "с (" 
                     << remaining_time / 60 << " минут)" << endl;
            }
        }
    });
    
    // Ждем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }
    
    // Останавливаем поток прогресса
    found = true;
    progress_thread.join();
    
    auto end_time = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    auto program_duration = chrono::duration_cast<chrono::seconds>(end_time - program_start);
    
    cout << "\n=== РЕЗУЛЬТАТ ===" << endl;
    cout << "Общее время выполнения: " << program_duration.count() << " секунд" << endl;
    cout << "Всего обработано комбинаций: " << total_processed << endl;
    
    if (!found) {
        cout << "Флаг не найден." << endl;
        cout << "Рекомендации:" << endl;
        cout << "1. Проверить другие форматы флага (SIBINTEK, CTF, FLAG и т.д.)" << endl;
        cout << "2. Расширить диапазоны стартовых позиций" << endl;
        cout << "3. Проверить правильность алгоритма дешифрования" << endl;
    }
    
    return 0;
}
