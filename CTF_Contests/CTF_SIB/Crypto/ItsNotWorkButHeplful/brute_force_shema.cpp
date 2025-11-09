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
#include <cmath>

using namespace std;

atomic<bool> found(false);
atomic<long long> total_processed(0);
mutex cout_mutex;

vector<vector<int>> twenty_position_rotors;  // 20-позиционные роторы
vector<vector<int>> six_position_rotors;     // 6-позиционные роторы
vector<int> ciphertext_num;

// Частоты букв в английском языке
const vector<double> english_freq = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015, // A-G
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749, // H-N
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758, // O-U
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074                   // V-Z
};

// Расширенный список английских слов
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

// Функция для применения ротора с учетом обратной связи
int applyRotor(int input, const vector<int>& rotor, int position, bool reverse = false) {
    if (reverse) {
        // Обратное преобразование - находим позицию входного символа в роторе
        for (int i = 0; i < rotor.size(); i++) {
            if (rotor[i] == input) {
                return i;
            }
        }
        return input;
    } else {
        // Прямое преобразование
        return rotor[(input + position) % rotor.size()];
    }
}

// Улучшенная функция дешифрования с учетом схемы устройства
string decryptWithFeedback(const vector<int>& rotor1, const vector<int>& rotor2, 
                          const vector<int>& rotor3, const vector<int>& rotor6,
                          int pos1, int pos2, int pos3, int pos6,
                          const vector<int>& input_plugboard, const vector<int>& output_plugboard) {
    
    vector<int> plaintext(ciphertext_num.size());
    int p1 = pos1, p2 = pos2, p3 = pos3, p6 = pos6;
    int n = ciphertext_num.size();
    
    for (int i = 0; i < n; i++) {
        int signal = ciphertext_num[i];
        
        // 1. Входная коммутационная панель
        signal = input_plugboard[signal];
        
        // 2. Проход через 20-позиционные переключатели
        signal = applyRotor(signal, rotor1, p1);
        signal = applyRotor(signal, rotor2, p2);
        signal = applyRotor(signal, rotor3, p3);
        
        // 3. Обратная связь от третьего переключателя к входной панели
        // (имитируем изменение состояния)
        int feedback = applyRotor(signal, rotor3, p3, true);
        
        // 4. Проход через 6-позиционный переключатель
        signal = applyRotor(signal, rotor6, p6);
        
        // 5. Выходная коммутационная панель
        signal = output_plugboard[signal];
        
        plaintext[i] = signal;
        
        // Вращение роторов (упрощенная модель)
        p1 = (p1 + 1) % 20;
        if (p1 == 0) {
            p2 = (p2 + 1) % 20;
            if (p2 == 0) {
                p3 = (p3 + 1) % 20;
                if (p3 == 0) {
                    p6 = (p6 + 1) % 6;
                }
            }
        }
    }
    
    return numToString(plaintext);
}

// Проверка на наличие флага
bool containsFlag(const string& text) {
    return text.find("SIBINTEK{") != string::npos;
}

// Проверка на английский текст
bool isLikelyEnglish(const string& text) {
    if (text.length() < 20) return false;
    
    // Проверка частот букв
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
    
    if (chi_squared > 150) return false;
    
    // Проверка английских слов
    string upper_text = text;
    transform(upper_text.begin(), upper_text.end(), upper_text.begin(), ::toupper);
    
    int word_matches = 0;
    for (const string& word : english_words) {
        if (upper_text.find(word) != string::npos) {
            word_matches++;
        }
    }
    
    return word_matches >= 3;
}

// Генерация случайной коммутационной панели
vector<int> generateRandomPlugboard() {
    vector<int> plugboard(26);
    for (int i = 0; i < 26; i++) {
        plugboard[i] = i;
    }
    random_shuffle(plugboard.begin(), plugboard.end());
    return plugboard;
}

void worker(int thread_id, int start_idx, int end_idx, 
            const vector<int>& twenty_indices, const vector<int>& six_indices,
            long long total_combinations) {
    
    auto thread_start = chrono::high_resolution_clock::now();
    long long local_processed = 0;
    
    // Простые коммутационные панели (можно расширить)
    vector<int> identity_plugboard(26);
    for (int i = 0; i < 26; i++) identity_plugboard[i] = i;
    
    vector<vector<int>> simple_plugboards = {identity_plugboard};
    
    for (int idx = start_idx; idx < end_idx && !found; idx++) {
        // Преобразуем линейный индекс в комбинации роторов
        int total_20 = twenty_indices.size();
        int total_6 = six_indices.size();
        
        int r1_idx = idx / (total_20 * total_20 * total_6);
        int remainder = idx % (total_20 * total_20 * total_6);
        int r2_idx = remainder / (total_20 * total_6);
        remainder = remainder % (total_20 * total_6);
        int r3_idx = remainder / total_6;
        int r6_idx = remainder % total_6;
        
        if (r1_idx >= total_20 || r2_idx >= total_20 || r3_idx >= total_20 || r6_idx >= total_6) {
            continue;
        }
        
        const auto& rotor1 = twenty_position_rotors[twenty_indices[r1_idx]];
        const auto& rotor2 = twenty_position_rotors[twenty_indices[r2_idx]];
        const auto& rotor3 = twenty_position_rotors[twenty_indices[r3_idx]];
        const auto& rotor6 = six_position_rotors[six_indices[r6_idx]];
        
        for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
            for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                    for (int pos6 = 1; pos6 <= 6 && !found; pos6++) {
                        for (const auto& input_pb : simple_plugboards) {
                            for (const auto& output_pb : simple_plugboards) {
                                string plaintext = decryptWithFeedback(
                                    rotor1, rotor2, rotor3, rotor6,
                                    pos1, pos2, pos3, pos6,
                                    input_pb, output_pb
                                );
                                
                                local_processed++;
                                total_processed++;
                                
                                if (containsFlag(plaintext)) {
                                    found = true;
                                    auto end_time = chrono::high_resolution_clock::now();
                                    auto total_elapsed = chrono::duration_cast<chrono::seconds>(end_time - thread_start);
                                    
                                    lock_guard<mutex> lock(cout_mutex);
                                    cout << "\n=== НАЙДЕН ФЛАГ! ===" << endl;
                                    cout << "Поток: " << thread_id << endl;
                                    cout << "20-позиционные роторы: " << twenty_indices[r1_idx] << ", " 
                                         << twenty_indices[r2_idx] << ", " << twenty_indices[r3_idx] << endl;
                                    cout << "6-позиционный ротор: " << six_indices[r6_idx] << endl;
                                    cout << "Позиции: " << pos1 << ", " << pos2 << ", " << pos3 << ", " << pos6 << endl;
                                    cout << "Обработано: " << local_processed << " комбинаций" << endl;
                                    cout << "Время: " << total_elapsed.count() << " секунд" << endl;
                                    cout << "Текст: " << plaintext << endl;
                                    
                                    size_t start = plaintext.find("SIBINTEK{");
                                    size_t end = plaintext.find('}', start);
                                    if (start != string::npos && end != string::npos) {
                                        cout << "ФЛАГ: " << plaintext.substr(start, end - start + 1) << endl;
                                    }
                                    return;
                                }
                                
                                if (local_processed % 10000 == 0) {
                                    auto now = chrono::high_resolution_clock::now();
                                    auto elapsed = chrono::duration_cast<chrono::seconds>(now - thread_start).count();
                                    
                                    lock_guard<mutex> lock(cout_mutex);
                                    double progress = 100.0 * (idx - start_idx) / (end_idx - start_idx);
                                    cout << "[Поток " << thread_id << "] Прогресс: " << fixed << setprecision(1) << progress 
                                         << "%, комбинаций: " << local_processed 
                                         << ", скорость: " << (elapsed > 0 ? local_processed / elapsed : local_processed) << " комб/с" << endl;
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

// Загрузка и классификация роторов
bool loadAndClassifyRotors(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Не удалось открыть файл с роторами." << endl;
        return false;
    }

    string line;
    while (getline(file, line)) {
        line.erase(remove_if(line.begin(), line.end(), [](char c) { return !isalpha(c); }), line.end());
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
                // Классифицируем роторы на 20-позиционные и 6-позиционные
                // Для простоты будем считать все роторы 20-позиционными,
                // а 6-позиционные сгенерируем искусственно
                twenty_position_rotors.push_back(rotor);
            }
        }
    }
    file.close();
    
    // Генерация 6-позиционных роторов
    for (int i = 0; i < 100; i++) {
        vector<int> rotor(6);
        for (int j = 0; j < 6; j++) rotor[j] = j;
        random_shuffle(rotor.begin(), rotor.end());
        six_position_rotors.push_back(rotor);
    }
    
    cout << "Загружено 20-позиционных роторов: " << twenty_position_rotors.size() << endl;
    cout << "Сгенерировано 6-позиционных роторов: " << six_position_rotors.size() << endl;
    
    return !twenty_position_rotors.empty() && !six_position_rotors.empty();
}

int main() {
    auto start_time = chrono::high_resolution_clock::now();
    
    if (!loadAndClassifyRotors("dict.txt")) {
        return 1;
    }
    
    string ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG";
    ciphertext_num = stringToNum(ciphertext);
    
    // Выбираем подмножества роторов для перебора
    vector<int> twenty_indices, six_indices;
    int max_twenty = min(50, (int)twenty_position_rotors.size());
    int max_six = min(10, (int)six_position_rotors.size());
    
    for (int i = 0; i < max_twenty; i++) twenty_indices.push_back(i);
    for (int i = 0; i < max_six; i++) six_indices.push_back(i);
    
    long long total_combinations = (long long)max_twenty * max_twenty * max_twenty * max_six * 10 * 10 * 16 * 6;
    
    cout << "\nНастройки перебора:" << endl;
    cout << "20-позиционные роторы: " << max_twenty << endl;
    cout << "6-позиционные роторы: " << max_six << endl;
    cout << "Общее количество комбинаций: " << total_combinations << endl;
    
    int num_threads = thread::hardware_concurrency();
    vector<thread> threads;
    int combinations_per_thread = total_combinations / num_threads;
    
    cout << "Запуск " << num_threads << " потоков..." << endl;
    
    for (int i = 0; i < num_threads; i++) {
        int start = i * combinations_per_thread;
        int end = (i == num_threads - 1) ? total_combinations : (i + 1) * combinations_per_thread;
        threads.emplace_back(worker, i, start, end, twenty_indices, six_indices, total_combinations);
    }
    
    // Мониторинг прогресса
    thread progress_thread([total_combinations, start_time]() {
        while (!found) {
            this_thread::sleep_for(chrono::seconds(10));
            auto now = chrono::high_resolution_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(now - start_time).count();
            long long processed = total_processed;
            double progress = 100.0 * processed / total_combinations;
            
            lock_guard<mutex> lock(cout_mutex);
            cout << "[ОБЩИЙ ПРОГРЕСС] " << fixed << setprecision(4) << progress 
                 << "%, обработано: " << processed << "/" << total_combinations
                 << ", время: " << elapsed << "с" 
                 << ", скорость: " << (elapsed > 0 ? processed / elapsed : processed) << " комб/с" << endl;
        }
    });
    
    for (auto& t : threads) t.join();
    found = true;
    progress_thread.join();
    
    auto end_time = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    
    cout << "\nОбщее время: " << total_duration.count() << " секунд" << endl;
    cout << "Всего обработано: " << total_processed << " комбинаций" << endl;
    
    if (!found) {
        cout << "Флаг не найден. Рекомендации:" << endl;
        cout << "1. Увеличить количество проверяемых роторов" << endl;
        cout << "2. Добавить больше вариантов коммутационных панелей" << endl;
        cout << "3. Уточнить алгоритм вращения роторов" << endl;
    }
    
    return 0;
}
