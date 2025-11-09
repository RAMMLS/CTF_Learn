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
#include <random>

using namespace std;

atomic<bool> found(false);
atomic<long long> total_processed(0);
mutex cout_mutex;

vector<vector<int>> twenty_position_rotors;
vector<vector<int>> six_position_rotors;
vector<int> ciphertext_num;

const vector<double> english_freq = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015,
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749,
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758,
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074
};

const vector<string> english_words = {
    "THE", "AND", "ING", "HER", "HAT", "HIS", "THA", "ERE", "FOR", "ENT",
    "ION", "TER", "WAS", "YOU", "ITH", "VER", "ALL", "WIT", "THI", "TIO"
};

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

// Упрощенная функция дешифрования без обратной связи для тестирования
string decryptSimple(const vector<int>& rotor1, const vector<int>& rotor2, 
                    const vector<int>& rotor3, const vector<int>& rotor6,
                    int pos1, int pos2, int pos3, int pos6) {
    
    vector<int> plaintext(ciphertext_num.size());
    int p1 = pos1, p2 = pos2, p3 = pos3, p6 = pos6;
    int n = ciphertext_num.size();
    
    for (int i = 0; i < n; i++) {
        int signal = ciphertext_num[i];
        
        // Проход через роторы (упрощенная версия)
        signal = rotor1[(signal + p1) % 20];
        signal = rotor2[(signal + p2) % 20];
        signal = rotor3[(signal + p3) % 20];
        signal = rotor6[(signal + p6) % 6];
        
        plaintext[i] = signal;
        
        // Вращение роторов
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

bool containsFlag(const string& text) {
    return text.find("SIBINTEK{") != string::npos;
}

bool isLikelyEnglish(const string& text) {
    if (text.length() < 20) return false;
    
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
    
    string upper_text = text;
    transform(upper_text.begin(), upper_text.end(), upper_text.begin(), ::toupper);
    
    int word_matches = 0;
    for (const string& word : english_words) {
        if (upper_text.find(word) != string::npos) {
            word_matches++;
        }
        if (word_matches >= 2) break; // Достаточно 2 совпадений
    }
    
    return word_matches >= 2;
}

void worker(int thread_id, int start_r1, int end_r1, 
            int total_twenty, int total_six) {
    
    auto thread_start = chrono::high_resolution_clock::now();
    long long local_processed = 0;
    long long last_logged = 0;
    
    // Ограничиваем диапазоны для безопасности
    start_r1 = max(0, start_r1);
    end_r1 = min(total_twenty, end_r1);
    
    for (int r1 = start_r1; r1 < end_r1 && !found; r1++) {
        for (int r2 = 0; r2 < total_twenty && !found; r2++) {
            if (r2 == r1) continue;
            
            for (int r3 = 0; r3 < total_twenty && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                
                for (int r6 = 0; r6 < total_six && !found; r6++) {
                    for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                        for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                            for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                                for (int pos6 = 1; pos6 <= 6 && !found; pos6++) {
                                    // Проверяем границы массивов
                                    if (r1 >= twenty_position_rotors.size() ||
                                        r2 >= twenty_position_rotors.size() ||
                                        r3 >= twenty_position_rotors.size() ||
                                        r6 >= six_position_rotors.size()) {
                                        continue;
                                    }
                                    
                                    string plaintext = decryptSimple(
                                        twenty_position_rotors[r1],
                                        twenty_position_rotors[r2], 
                                        twenty_position_rotors[r3],
                                        six_position_rotors[r6],
                                        pos1, pos2, pos3, pos6
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
                                        cout << "Роторы: " << r1 << ", " << r2 << ", " << r3 << ", " << r6 << endl;
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
                                    
                                    // Логирование прогресса каждые 10000 комбинаций
                                    if (local_processed - last_logged >= 10000) {
                                        last_logged = local_processed;
                                        auto now = chrono::high_resolution_clock::now();
                                        auto elapsed = chrono::duration_cast<chrono::seconds>(now - thread_start).count();
                                        
                                        if (elapsed > 0) {
                                            lock_guard<mutex> lock(cout_mutex);
                                            cout << "[Поток " << thread_id << "] Ротор1: " << r1 
                                                 << ", комбинаций: " << local_processed 
                                                 << ", скорость: " << (local_processed / elapsed) << " комб/с" << endl;
                                        }
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

bool loadAndClassifyRotors(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        // Пробуем разные имена файлов
        vector<string> filenames = {"dict.txt", "dict (1).txt", "dict(1).txt"};
        for (const auto& fname : filenames) {
            file.open(fname);
            if (file.is_open()) {
                cout << "Открыт файл: " << fname << endl;
                break;
            }
        }
        
        if (!file.is_open()) {
            cout << "Не удалось открыть файл с роторами" << endl;
            return false;
        }
    }

    string line;
    int loaded = 0;
    
    while (getline(file, line) && loaded < 100) { // Ограничиваем количество роторов
        // Очистка строки
        line.erase(remove_if(line.begin(), line.end(), [](char c) { 
            return !isalpha(c); 
        }), line.end());
        
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
                twenty_position_rotors.push_back(rotor);
                loaded++;
            }
        }
    }
    file.close();
    
    // Создаем 6-позиционные роторы
    random_device rd;
    mt19937 gen(rd());
    
    for (int i = 0; i < 20; i++) {
        vector<int> rotor(6);
        for (int j = 0; j < 6; j++) rotor[j] = j;
        shuffle(rotor.begin(), rotor.end(), gen);
        six_position_rotors.push_back(rotor);
    }
    
    cout << "Загружено 20-позиционных роторов: " << twenty_position_rotors.size() << endl;
    cout << "Создано 6-позиционных роторов: " << six_position_rotors.size() << endl;
    
    return !twenty_position_rotors.empty() && !six_position_rotors.empty();
}

int main() {
    auto start_time = chrono::high_resolution_clock::now();
    
    cout << "Загрузка роторов..." << endl;
    if (!loadAndClassifyRotors("dict.txt")) {
        return 1;
    }
    
    // Подготовка шифртекста
    string ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG";
    ciphertext_num = stringToNum(ciphertext);
    
    // Ограничиваем количество роторов для перебора
    int total_twenty = min(10, (int)twenty_position_rotors.size());
    int total_six = min(5, (int)six_position_rotors.size());
    
    cout << "\nНастройки перебора:" << endl;
    cout << "20-позиционные роторы: " << total_twenty << endl;
    cout << "6-позиционные роторы: " << total_six << endl;
    
    // Расчет комбинаций
    long long total_combinations = (long long)total_twenty * 
                                  (total_twenty - 1) * 
                                  (total_twenty - 2) * 
                                  total_six * 
                                  10 * 10 * 16 * 6;
    
    cout << "Общее количество комбинаций: " << total_combinations << endl;
    
    int num_threads = min(8, (int)thread::hardware_concurrency()); // Ограничиваем потоки
    cout << "Используется потоков: " << num_threads << endl;
    
    vector<thread> threads;
    int rotors_per_thread = total_twenty / num_threads;
    
    cout << "Запуск потоков..." << endl;
    
    for (int i = 0; i < num_threads; i++) {
        int start = i * rotors_per_thread;
        int end = (i == num_threads - 1) ? total_twenty : (i + 1) * rotors_per_thread;
        
        cout << "Поток " << i << ": роторы " << start << " - " << end-1 << endl;
        threads.emplace_back(worker, i, start, end, total_twenty, total_six);
    }
    
    // Поток для мониторинга прогресса
    thread progress_thread([total_combinations, start_time]() {
        while (!found) {
            this_thread::sleep_for(chrono::seconds(15));
            
            auto now = chrono::high_resolution_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(now - start_time).count();
            long long processed = total_processed;
            
            if (elapsed > 0) {
                double progress = 100.0 * processed / total_combinations;
                long long speed = processed / elapsed;
                
                lock_guard<mutex> lock(cout_mutex);
                cout << "\n[ОБЩИЙ ПРОГРЕСС] " << fixed << setprecision(4) << progress 
                     << "%, обработано: " << processed << "/" << total_combinations
                     << ", время: " << elapsed << "с" 
                     << ", скорость: " << speed << " комб/с" << endl;
                     
                if (progress > 0) {
                    long long remaining = (100.0 - progress) * elapsed / progress;
                    cout << "[ОЦЕНКА] Примерное время до завершения: " << remaining << "с (" 
                         << remaining / 60 << " минут)" << endl;
                }
            }
        }
    });
    
    // Ожидаем завершения потоков
    for (auto& t : threads) {
        t.join();
    }
    
    found = true;
    progress_thread.join();
    
    auto end_time = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    
    cout << "\n=== РЕЗУЛЬТАТ ===" << endl;
    cout << "Общее время: " << total_duration.count() << " секунд" << endl;
    cout << "Всего обработано: " << total_processed << " комбинаций" << endl;
    
    if (!found) {
        cout << "Флаг не найден в заданных пределах." << endl;
        cout << "Рекомендации:" << endl;
        cout << "1. Увеличить количество проверяемых роторов" << endl;
        cout << "2. Расширить диапазоны позиций" << endl;
        cout << "3. Проверить другие возможные форматы флага" << endl;
    }
    
    return 0;
}
