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

using namespace std;

atomic<bool> found(false);
atomic<long long> total_processed(0);
mutex cout_mutex;

vector<string> rotors;
string ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG";

// Ультра-оптимизированная функция дешифрования
string decryptUltra(const string& rotor1, const string& rotor2, const string& rotor3,
                   int pos1, int pos2, int pos3) {
    string plaintext;
    plaintext.reserve(ciphertext.length());
    
    int p1 = pos1, p2 = pos2, p3 = pos3;
    
    for (char c : ciphertext) {
        if (c < 'A' || c > 'Z') {
            plaintext += c;
            continue;
        }
        
        int signal = c - 'A';
        
        // Прямой проход через роторы
        signal = rotor1[(signal + p1) % 26] - 'A';
        signal = rotor2[(signal + p2) % 26] - 'A';
        signal = rotor3[(signal + p3) % 26] - 'A';
        
        plaintext += char(signal + 'A');
        
        // Вращение роторов
        p1 = (p1 + 1) % 26;
        if (p1 == 0) {
            p2 = (p2 + 1) % 26;
            if (p2 == 0) {
                p3 = (p3 + 1) % 26;
            }
        }
    }
    
    return plaintext;
}

// Быстрая проверка на флаг
bool containsFlag(const string& text) {
    return text.find("SIBINTEK{") != string::npos;
}

void ultraWorker(int thread_id, int start_idx, int end_idx, int total_rotors) {
    auto start_time = chrono::high_resolution_clock::now();
    long long local_processed = 0;
    const int report_interval = 500000; // Отчет каждые 500K комбинаций
    
    for (int r1 = start_idx; r1 < end_idx && !found; r1++) {
        const string& rotor1 = rotors[r1];
        
        for (int r2 = 0; r2 < total_rotors && !found; r2++) {
            if (r2 == r1) continue;
            const string& rotor2 = rotors[r2];
            
            for (int r3 = 0; r3 < total_rotors && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                const string& rotor3 = rotors[r3];
                
                // Только допустимые позиции согласно hint.txt
                for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                    for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                        for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                            string plaintext = decryptUltra(rotor1, rotor2, rotor3, pos1, pos2, pos3);
                            local_processed++;
                            total_processed++;
                            
                            if (containsFlag(plaintext)) {
                                found = true;
                                auto end_time = chrono::high_resolution_clock::now();
                                auto elapsed = chrono::duration_cast<chrono::seconds>(end_time - start_time);
                                
                                lock_guard<mutex> lock(cout_mutex);
                                cout << "\n🎉 ФЛАГ НАЙДЕН! 🎉" << endl;
                                cout << "Поток: " << thread_id << endl;
                                cout << "Роторы: " << r1 << ", " << r2 << ", " << r3 << endl;
                                cout << "Позиции: " << pos1 << ", " << pos2 << ", " << pos3 << endl;
                                cout << "Время: " << elapsed.count() << " секунд" << endl;
                                cout << "Обработано: " << local_processed << " комбинаций" << endl;
                                cout << "Полный текст: " << plaintext << endl;
                                
                                // Автоматическое извлечение флага
                                size_t start = plaintext.find("SIBINTEK{");
                                if (start != string::npos) {
                                    size_t end = plaintext.find('}', start);
                                    if (end != string::npos) {
                                        cout << "ФЛАГ: " << plaintext.substr(start, end - start + 1) << endl;
                                    }
                                }
                                return;
                            }
                            
                            if (local_processed % report_interval == 0) {
                                auto now = chrono::high_resolution_clock::now();
                                auto elapsed_sec = chrono::duration_cast<chrono::seconds>(now - start_time).count();
                                long long speed = (elapsed_sec > 0) ? (local_processed / elapsed_sec) : local_processed;
                                
                                lock_guard<mutex> lock(cout_mutex);
                                cout << "[П" << thread_id << "] R1:" << r1 << " К:" << local_processed 
                                     << " С:" << speed << "/с" << endl;
                            }
                        }
                    }
                }
            }
        }
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    
    lock_guard<mutex> lock(cout_mutex);
    cout << "[П" << thread_id << "] Завершен. " << local_processed 
         << " комбинаций за " << elapsed.count() << "с" << endl;
}

bool loadRotorsSimple(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        // Пробуем найти файл
        vector<string> attempts = {"dict.txt", "dict (1).txt", "dict(1).txt", "./dict.txt", "./dict (1).txt"};
        for (const auto& fname : attempts) {
            file.open(fname);
            if (file.is_open()) {
                cout << "Файл найден: " << fname << endl;
                break;
            }
        }
        if (!file.is_open()) {
            cout << "Файл с роторами не найден!" << endl;
            return false;
        }
    }

    string line;
    int count = 0;
    while (getline(file, line)) {
        // Быстрая очистка и проверка
        if (line.length() == 26) {
            bool valid = true;
            for (char c : line) {
                if (c < 'A' || c > 'Z') {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                rotors.push_back(line);
                count++;
                // Ограничиваем количество для теста - уберем ограничение или увеличим
                if (count >= 500) break; // Увеличил до 500 роторов
            }
        }
    }
    file.close();
    
    cout << "Загружено роторов: " << count << endl;
    return count > 0;
}

int main() {
    auto program_start = chrono::high_resolution_clock::now();
    
    cout << "🚀 Загрузка роторов..." << endl;
    if (!loadRotorsSimple("dict.txt")) {
        return 1;
    }
    
    int total_rotors = rotors.size();
    int num_threads = thread::hardware_concurrency();
    
    // УВЕЛИЧИВАЕМ количество проверяемых роторов
    int test_rotors = total_rotors; // Используем все загруженные роторы
    
    cout << "\n⚡ Настройки брутфорса:" << endl;
    cout << "Роторов для проверки: " << test_rotors << endl;
    cout << "Потоки: " << num_threads << endl;
    
    long long total_combinations = (long long)test_rotors * (test_rotors-1) * (test_rotors-2) * 10 * 10 * 16;
    cout << "Всего комбинаций: " << total_combinations << endl;
    cout << "Старт..." << endl;
    
    vector<thread> threads;
    int rotors_per_thread = test_rotors / num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        int start = i * rotors_per_thread;
        int end = (i == num_threads - 1) ? test_rotors : (i + 1) * rotors_per_thread;
        cout << "Поток " << i << ": роторы " << start << "-" << end-1 << endl;
        threads.emplace_back(ultraWorker, i, start, end, test_rotors);
    }
    
    // Мониторинг прогресса
    thread monitor([total_combinations, program_start]() {
        while (!found) {
            this_thread::sleep_for(chrono::seconds(30));
            
            auto now = chrono::high_resolution_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(now - program_start).count();
            long long processed = total_processed;
            double progress = 100.0 * processed / total_combinations;
            long long speed = (elapsed > 0) ? (processed / elapsed) : processed;
            
            lock_guard<mutex> lock(cout_mutex);
            cout << "\n📊 ОБЩИЙ ПРОГРЕСС: " << fixed << setprecision(4) << progress << "%" 
                 << " (" << processed << "/" << total_combinations << ")" 
                 << " Скорость: " << speed << "/с" 
                 << " Время: " << elapsed << "с" << endl;
                 
            if (progress > 0.1) {
                long long remaining = (100.0 - progress) * elapsed / progress;
                cout << "⏱️  Осталось: ~" << remaining << "с (" << remaining/60 << " минут)" << endl;
            }
        }
    });
    
    for (auto& t : threads) t.join();
    found = true;
    monitor.join();
    
    auto program_end = chrono::high_resolution_clock::now();
    auto total_time = chrono::duration_cast<chrono::seconds>(program_end - program_start);
    
    cout << "\n=== РЕЗУЛЬТАТ ===" << endl;
    cout << "Общее время: " << total_time.count() << " секунд" << endl;
    cout << "Обработано комбинаций: " << total_processed << endl;
    
    if (!found) {
        cout << "❌ Флаг не найден в текущем диапазоне." << endl;
        cout << "Рекомендации:" << endl;
        cout << "1. Увеличить количество загружаемых роторов (сейчас 500)" << endl;
        cout << "2. Проверить другие форматы флага" << endl;
        cout << "3. Уточнить алгоритм шифрования" << endl;
    }
    
    return 0;
}
