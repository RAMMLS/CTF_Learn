#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

using namespace std;

atomic<bool> found(false);
atomic<int> total_processed(0);
mutex cout_mutex;

vector<string> rotors;
string ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG";

// Быстрая функция дешифрования
string fastDecrypt(const string& rotor1, const string& rotor2, const string& rotor3, 
                  int pos1, int pos2, int pos3) {
    string result;
    result.reserve(ciphertext.length());
    
    int p1 = pos1, p2 = pos2, p3 = pos3;
    
    for (char c : ciphertext) {
        if (!isalpha(c)) {
            result += c;
            continue;
        }
        
        int char_idx = c - 'A';
        
        // Обратный проход через роторы
        char_idx = (char_idx + p3) % 26;
        char_idx = rotor3[char_idx] - 'A';
        char_idx = (char_idx - p3 + 26) % 26;
        
        char_idx = (char_idx + p2) % 26;
        char_idx = rotor2[char_idx] - 'A';
        char_idx = (char_idx - p2 + 26) % 26;
        
        char_idx = (char_idx + p1) % 26;
        char_idx = rotor1[char_idx] - 'A';
        char_idx = (char_idx - p1 + 26) % 26;
        
        result += char_idx + 'A';
        
        // Вращение роторов
        p1 = (p1 + 1) % 26;
        if (p1 == 0) {
            p2 = (p2 + 1) % 26;
            if (p2 == 0) {
                p3 = (p3 + 1) % 26;
            }
        }
    }
    
    return result;
}

// Быстрая проверка на флаг (только начало текста)
bool quickCheckFlag(const string& text) {
    // Проверяем только первые 200 символов для скорости
    if (text.length() < 50) return false;
    
    string start = text.substr(0, 200);
    return start.find("SIBINTEK{") != string::npos;
}

void optimizedWorker(int start_r1, int end_r1) {
    int local_processed = 0;
    const int total_rotors = rotors.size();
    
    for (int r1 = start_r1; r1 < end_r1 && !found; r1++) {
        for (int r2 = 0; r2 < total_rotors && !found; r2++) {
            if (r2 == r1) continue;
            
            for (int r3 = 0; r3 < total_rotors && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                
                // Только ключевые позиции из hint.txt
                for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                    for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                        for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                            local_processed++;
                            total_processed++;
                            
                            string plaintext = fastDecrypt(rotors[r1], rotors[r2], rotors[r3], 
                                                         pos1, pos2, pos3);
                            
                            if (quickCheckFlag(plaintext)) {
                                found = true;
                                lock_guard<mutex> lock(cout_mutex);
                                cout << "\n=== ФЛАГ НАЙДЕН! ===" << endl;
                                cout << "Роторы: " << r1 << ", " << r2 << ", " << r3 << endl;
                                cout << "Позиции: " << pos1 << ", " << pos2 << ", " << pos3 << endl;
                                cout << "Текст: " << plaintext << endl;
                                
                                size_t start = plaintext.find("SIBINTEK{");
                                size_t end = plaintext.find('}', start);
                                if (start != string::npos && end != string::npos) {
                                    cout << "ФЛАГ: " << plaintext.substr(start, end - start + 1) << endl;
                                }
                                return;
                            }
                        }
                    }
                }
            }
        }
        
        // Прогресс каждые 10 роторов
        if (r1 % 10 == 0) {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Проверено роторов R1: " << r1 << "/" << end_r1 
                 << ", комбинаций: " << local_processed << endl;
        }
    }
}

bool loadRotorsSimple(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        // Попробуем другие имена
        vector<string> alternatives = {"dict.txt", "dict (1).txt", "dict(1).txt"};
        for (const auto& alt : alternatives) {
            file.open(alt);
            if (file.is_open()) {
                cout << "Открыт файл: " << alt << endl;
                break;
            }
        }
    }
    
    if (!file.is_open()) {
        cout << "Не удалось открыть файл с роторами!" << endl;
        return false;
    }
    
    string line;
    while (getline(file, line)) {
        if (line.length() == 26) {
            // Простая проверка - все символы A-Z
            bool valid = true;
            for (char c : line) {
                if (!isalpha(c)) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                // Приводим к верхнему регистру
                transform(line.begin(), line.end(), line.begin(), ::toupper);
                rotors.push_back(line);
            }
        }
    }
    
    file.close();
    cout << "Загружено " << rotors.size() << " роторов" << endl;
    return !rotors.empty();
}

int main() {
    auto start_time = chrono::high_resolution_clock::now();
    
    cout << "Загрузка роторов..." << endl;
    if (!loadRotorsSimple("dict.txt")) {
        return 1;
    }
    
    // ОГРАНИЧИВАЕМ количество роторов для теста
    int TEST_ROTORS = min(30, (int)rotors.size());
    cout << "Тестируем " << TEST_ROTORS << " роторов" << endl;
    
    // Расчет комбинаций
    long long combinations = (long long)TEST_ROTORS * (TEST_ROTORS-1) * (TEST_ROTORS-2) * 10 * 10 * 16;
    cout << "Примерное количество комбинаций: " << combinations << endl;
    
    int num_threads = thread::hardware_concurrency();
    cout << "Используем " << num_threads << " потоков" << endl;
    
    vector<thread> threads;
    int rotors_per_thread = TEST_ROTORS / num_threads;
    
    // Запускаем потоки
    for (int i = 0; i < num_threads; i++) {
        int start = i * rotors_per_thread;
        int end = (i == num_threads - 1) ? TEST_ROTORS : (i + 1) * rotors_per_thread;
        threads.emplace_back(optimizedWorker, start, end);
    }
    
    // Мониторинг прогресса
    thread progress_thread([start_time]() {
        auto last_time = start_time;
        int last_processed = 0;
        
        while (!found) {
            this_thread::sleep_for(chrono::seconds(30));
            
            auto now = chrono::high_resolution_clock::now();
            auto total_elapsed = chrono::duration_cast<chrono::seconds>(now - start_time);
            int current_processed = total_processed;
            int delta_processed = current_processed - last_processed;
            auto delta_time = chrono::duration_cast<chrono::seconds>(now - last_time);
            
            lock_guard<mutex> lock(cout_mutex);
            cout << "[ПРОГРЕСС] Обработано: " << current_processed 
                 << ", скорость: " << (delta_time.count() > 0 ? delta_processed / delta_time.count() : 0) << " комб/с"
                 << ", время: " << total_elapsed.count() << "с" << endl;
                 
            last_processed = current_processed;
            last_time = now;
        }
    });
    
    // Ожидаем завершения
    for (auto& t : threads) {
        t.join();
    }
    
    found = true;
    progress_thread.join();
    
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    
    cout << "\n=== РЕЗУЛЬТАТ ===" << endl;
    cout << "Общее время: " << duration.count() << " секунд" << endl;
    cout << "Всего комбинаций: " << total_processed << endl;
    
    if (!found) {
        cout << "Флаг не найден в тестовом диапазоне." << endl;
        cout << "Попробуйте увеличить TEST_ROTORS в коде." << endl;
    }
    
    return 0;
}
