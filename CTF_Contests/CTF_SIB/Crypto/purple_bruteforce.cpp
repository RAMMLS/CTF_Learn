#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <cmath>
#include <map>
#include <set>

using namespace std;

atomic<bool> found(false);
atomic<long long> total_processed(0);
mutex cout_mutex;

vector<vector<int>> consonant_rotors;
vector<vector<int>> vowel_rotors;
vector<int> ciphertext_num;

const string consonants = "BCDFGHJKLMNPQRSTVWXZ"; // 20 согласных
const string vowels = "AEIOUY"; // 6 гласных

// Частоты букв в английском языке
const vector<double> english_freq = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015,
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749,
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758,
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074
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

string numToString(const vector<int>& nums) {
    string result;
    for (int num : nums) {
        result += char(num + 'A');
    }
    return result;
}

// Упрощенное создание роторов - берем первые 20 символов для согласных и первые 6 для гласных
vector<int> createConsonantRotor(const string& rotor_str) {
    vector<int> rotor(20);
    for (int i = 0; i < 20; i++) {
        char c = rotor_str[i];
        size_t pos = consonants.find(c);
        if (pos != string::npos) {
            rotor[i] = pos;
        } else {
            // Если символ не согласная, используем модульную арифметику
            rotor[i] = (c - 'A') % 20;
        }
    }
    return rotor;
}

vector<int> createVowelRotor(const string& rotor_str) {
    vector<int> rotor(6);
    for (int i = 0; i < 6; i++) {
        char c = rotor_str[i + 20]; // Берем символы после первых 20
        size_t pos = vowels.find(c);
        if (pos != string::npos) {
            rotor[i] = pos;
        } else {
            // Если символ не гласная, используем модульную арифметику
            rotor[i] = (c - 'A') % 6;
        }
    }
    return rotor;
}

// Альтернативный метод создания роторов - используем все 26 символов
vector<int> createRotorFromString(const string& rotor_str, int size, const string& alphabet) {
    vector<int> rotor(size);
    for (int i = 0; i < size; i++) {
        char c = rotor_str[i % rotor_str.length()];
        size_t pos = alphabet.find(c);
        if (pos != string::npos) {
            rotor[i] = pos;
        } else {
            rotor[i] = i % alphabet.length();
        }
    }
    return rotor;
}

// Улучшенная функция дешифрования Purple
string purpleDecrypt(const vector<int>& rotor1, const vector<int>& rotor2, const vector<int>& rotor3, 
                    const vector<int>& vowel_rotor, int pos1, int pos2, int pos3, int pos_v) {
    string plaintext;
    int p1 = pos1, p2 = pos2, p3 = pos3, pv = pos_v;
    
    for (int c : ciphertext_num) {
        char current_char = c + 'A';
        
        if (consonants.find(current_char) != string::npos) {
            // Обработка согласных через 20-позиционные роторы
            int idx = consonants.find(current_char);
            
            // Обратный проход через роторы
            idx = (idx + p3) % 20;
            idx = rotor3[idx];
            idx = (idx - p3 + 20) % 20;
            
            idx = (idx + p2) % 20;
            idx = rotor2[idx];
            idx = (idx - p2 + 20) % 20;
            
            idx = (idx + p1) % 20;
            idx = rotor1[idx];
            idx = (idx - p1 + 20) % 20;
            
            plaintext += consonants[idx];
            
            // Вращение роторов для согласных
            p1 = (p1 + 1) % 20;
            if (p1 == 0) {
                p2 = (p2 + 1) % 20;
                if (p2 == 0) {
                    p3 = (p3 + 1) % 20;
                }
            }
        } else if (vowels.find(current_char) != string::npos) {
            // Обработка гласных через 6-позиционный ротор
            int idx = vowels.find(current_char);
            
            idx = (idx + pv) % 6;
            idx = vowel_rotor[idx];
            idx = (idx - pv + 6) % 6;
            
            plaintext += vowels[idx];
            
            // Вращение ротора для гласных
            pv = (pv + 1) % 6;
        } else {
            plaintext += current_char;
        }
    }
    
    return plaintext;
}

// Улучшенная проверка на английский текст
bool isLikelyEnglish(const string& text, double threshold = 0.7) {
    if (text.length() < 50) return false;
    
    // Проверка частот букв (хи-квадрат)
    vector<int> counts(26, 0);
    int total_letters = 0;
    
    for (char c : text) {
        if (c >= 'A' && c <= 'Z') {
            counts[c - 'A']++;
            total_letters++;
        }
    }
    
    if (total_letters < 20) return false;
    
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

// Проверка на наличие флага
bool containsFlag(const string& text) {
    return text.find("SIBINTEK{") != string::npos;
}

// Извлечение флага
string extractFlag(const string& text) {
    size_t start = text.find("SIBINTEK{");
    if (start == string::npos) return "";
    
    size_t end = text.find('}', start);
    if (end == string::npos) return "";
    
    return text.substr(start, end - start + 1);
}

void purple_worker(int thread_id, int start_r, int end_r, int total_rotors) {
    auto thread_start = chrono::high_resolution_clock::now();
    long long local_processed = 0;
    
    for (int r1 = start_r; r1 < end_r && !found; r1++) {
        for (int r2 = 0; r2 < total_rotors && !found; r2++) {
            if (r2 == r1) continue;
            
            for (int r3 = 0; r3 < total_rotors && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                
                for (int rv = 0; rv < total_rotors && !found; rv++) {
                    for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                        for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                            for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                                for (int pos_v = 1; pos_v <= 6 && !found; pos_v++) {
                                    string plaintext = purpleDecrypt(
                                        consonant_rotors[r1], consonant_rotors[r2], consonant_rotors[r3],
                                        vowel_rotors[rv], pos1, pos2, pos3, pos_v
                                    );
                                    
                                    local_processed++;
                                    total_processed++;
                                    
                                    if (containsFlag(plaintext)) {
                                        found = true;
                                        auto end_time = chrono::high_resolution_clock::now();
                                        auto total_elapsed = chrono::duration_cast<chrono::seconds>(end_time - thread_start);
                                        
                                        lock_guard<mutex> lock(cout_mutex);
                                        cout << "\n=== PURPLE ФЛАГ НАЙДЕН! ===" << endl;
                                        cout << "Поток: " << thread_id << endl;
                                        cout << "Согласные роторы: " << r1 << ", " << r2 << ", " << r3 << endl;
                                        cout << "Гласный ротор: " << rv << endl;
                                        cout << "Позиции: " << pos1 << ", " << pos2 << ", " << pos3 << ", " << pos_v << endl;
                                        cout << "Обработано комбинаций: " << local_processed << endl;
                                        cout << "Время работы: " << total_elapsed.count() << " секунд" << endl;
                                        cout << "Текст: " << plaintext << endl;
                                        
                                        string flag = extractFlag(plaintext);
                                        if (!flag.empty()) {
                                            cout << "ФЛАГ: " << flag << endl;
                                        }
                                        return;
                                    }
                                    
                                    // Проверка на английский текст
                                    if (isLikelyEnglish(plaintext)) {
                                        lock_guard<mutex> lock(cout_mutex);
                                        cout << "[ВОЗМОЖНЫЙ ТЕКСТ] Поток " << thread_id 
                                             << ", роторы: " << r1 << "," << r2 << "," << r3 << "," << rv
                                             << ", позиции: " << pos1 << "," << pos2 << "," << pos3 << "," << pos_v << endl;
                                        cout << "Текст: " << plaintext.substr(0, 100) << "..." << endl;
                                    }
                                    
                                    // Логирование прогресса
                                    if (local_processed % 10000 == 0) {
                                        auto now = chrono::high_resolution_clock::now();
                                        auto elapsed = chrono::duration_cast<chrono::seconds>(now - thread_start).count();
                                        
                                        lock_guard<mutex> lock(cout_mutex);
                                        cout << "[Поток " << thread_id << "] Ротор1: " << r1 
                                             << ", комбинаций: " << local_processed 
                                             << ", скорость: " << (elapsed > 0 ? local_processed / elapsed : local_processed) << " комб/с" << endl;
                                    }
                                }
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
    cout << "[Поток " << thread_id << "] Завершен. Обработано: " << local_processed 
         << " комбинаций, время: " << thread_elapsed.count() << "с" << endl;
}

bool loadAndPrepareRotors(const string& filename) {
    vector<string> possible_filenames = {
        "dict.txt", 
        "dict (1).txt", 
        "dict(1).txt",
        "./dict.txt",
        "../dict.txt",
        "../../dict.txt"
    };

    ifstream file;
    string used_filename;
    
    // Пробуем открыть файл
    for (const auto& fname : possible_filenames) {
        file.open(fname);
        if (file.is_open()) {
            used_filename = fname;
            cout << "Успешно открыт файл: " << fname << endl;
            break;
        }
    }

    if (!file.is_open()) {
        cout << "Не удалось открыть файл с роторами. Проверенные имена: ";
        for (const auto& fname : possible_filenames) {
            cout << fname << " ";
        }
        cout << endl;
        return false;
    }

    string line;
    int loaded = 0;
    int line_num = 0;
    
    while (getline(file, line)) {
        line_num++;
        
        // Очистка строки от не-буквенных символов
        string cleaned_line;
        for (char c : line) {
            if (isalpha(c)) {
                cleaned_line += toupper(c);
            }
        }
        
        // Проверяем длину строки
        if (cleaned_line.length() != 26) {
            cout << "Пропущена строка " << line_num << ": неверная длина " << cleaned_line.length() << endl;
            continue;
        }
        
        // Создаем роторы
        vector<int> cons_rotor = createConsonantRotor(cleaned_line);
        vector<int> vowel_rotor = createVowelRotor(cleaned_line);
        
        consonant_rotors.push_back(cons_rotor);
        vowel_rotors.push_back(vowel_rotor);
        loaded++;
        
        // Выводим прогресс загрузки
        if (loaded % 100 == 0) {
            cout << "Загружено " << loaded << " роторов..." << endl;
        }
    }
    
    file.close();
    
    cout << "Загружено и подготовлено " << loaded << " роторов" << endl;
    cout << "Создано " << consonant_rotors.size() << " 20-позиционных роторов для согласных" << endl;
    cout << "Создано " << vowel_rotors.size() << " 6-позиционных роторов для гласных" << endl;
    
    // Диагностика первого ротора
    if (!consonant_rotors.empty()) {
        cout << "Первый согласный ротор: ";
        for (int i = 0; i < min(5, (int)consonant_rotors[0].size()); i++) {
            cout << consonant_rotors[0][i] << " ";
        }
        cout << endl;
    }
    
    if (!vowel_rotors.empty()) {
        cout << "Первый гласный ротор: ";
        for (int i = 0; i < min(3, (int)vowel_rotors[0].size()); i++) {
            cout << vowel_rotors[0][i] << " ";
        }
        cout << endl;
    }
    
    return !consonant_rotors.empty() && !vowel_rotors.empty();
}

int main() {
    auto program_start = chrono::high_resolution_clock::now();
    
    cout << "Загрузка и подготовка роторов для Purple..." << endl;
    if (!loadAndPrepareRotors("dict.txt")) {
        cout << "Пробуем альтернативный метод загрузки..." << endl;
        
        // Альтернативный метод: создаем тестовые роторы
        string test_rotor = "BCDFGHJKLMNPQRSTVWXZAEIOUY"; // 20 согласных + 6 гласных
        
        for (int i = 0; i < 100; i++) {
            // Создаем простые тестовые роторы
            consonant_rotors.push_back(createConsonantRotor(test_rotor));
            vowel_rotors.push_back(createVowelRotor(test_rotor));
            
            // Немного перемешиваем для разнообразия
            next_permutation(test_rotor.begin(), test_rotor.end());
        }
        
        cout << "Создано " << consonant_rotors.size() << " тестовых роторов" << endl;
    }
    
    // Подготовка шифртекста
    string ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG";
    ciphertext_num = stringToNum(ciphertext);
    
    int total_rotors = min(50, (int)consonant_rotors.size()); // Ограничиваем для теста
    int num_threads = thread::hardware_concurrency();
    
    cout << "\nНастройки Purple брутфорса:" << endl;
    cout << "Тестируем роторов: " << total_rotors << endl;
    cout << "Потоков: " << num_threads << endl;
    
    // Расчет комбинаций
    long long total_combinations = (long long)total_rotors * 
                                  (total_rotors - 1) * 
                                  (total_rotors - 2) * 
                                  total_rotors * 
                                  10 * 10 * 16 * 6;
    
    cout << "Общее количество комбинаций: " << total_combinations << endl;
    
    vector<thread> threads;
    int rotors_per_thread = total_rotors / num_threads;
    
    auto start_time = chrono::high_resolution_clock::now();
    
    cout << "Запуск потоков..." << endl;
    for (int i = 0; i < num_threads; i++) {
        int start = i * rotors_per_thread;
        int end = (i == num_threads - 1) ? total_rotors : (i + 1) * rotors_per_thread;
        cout << "Поток " << i << ": роторы " << start << " - " << end-1 << endl;
        threads.emplace_back(purple_worker, i, start, end, total_rotors);
    }
    
    // Поток для мониторинга прогресса
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
            cout << "\n[ОБЩИЙ ПРОГРЕСС] " << fixed << progress 
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
    
    for (auto& t : threads) {
        t.join();
    }
    
    found = true;
    progress_thread.join();
    
    auto end_time = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    auto program_duration = chrono::duration_cast<chrono::seconds>(end_time - program_start);
    
    cout << "\n=== РЕЗУЛЬТАТ ===" << endl;
    cout << "Общее время выполнения: " << program_duration.count() << " секунд" << endl;
    cout << "Всего обработано комбинаций: " << total_processed << endl;
    
    if (!found) {
        cout << "Флаг не найден в заданных пределах." << endl;
        cout << "Рекомендации:" << endl;
        cout << "1. Увеличить количество проверяемых роторов" << endl;
        cout << "2. Проверить другие возможные алгоритмы дешифрования" << endl;
        cout << "3. Уточнить параметры Purple-машины" << endl;
    }
    
    return 0;
}
