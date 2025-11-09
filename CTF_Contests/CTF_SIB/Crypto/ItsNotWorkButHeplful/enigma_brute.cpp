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

// Функция для проверки существования файла
bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

// Функция загрузки роторов
bool loadRotors(const string& filename) {
    cout << "Поиск файла с роторами..." << endl;
    
    vector<string> possible_names = {
        "dict.txt", "dict (1).txt", "dict(1).txt", 
        "dict", "dictionary.txt", "rotors.txt"
    };
    
    string actual_filename;
    for (const auto& name : possible_names) {
        if (fileExists(name)) {
            actual_filename = name;
            cout << "Найден файл: " << name << endl;
            break;
        }
    }
    
    if (actual_filename.empty()) {
        cout << "Файл с роторами не найден!" << endl;
        cout << "Разместите файл с роторами в той же директории, что и программа." << endl;
        return false;
    }

    ifstream file(actual_filename);
    if (!file.is_open()) {
        cout << "Ошибка открытия файла: " << actual_filename << endl;
        return false;
    }

    string line;
    int valid_rotors = 0;
    
    while (getline(file, line)) {
        // Очистка строки от не-алфавитных символов
        string cleaned_line;
        for (char c : line) {
            if (isalpha(c)) {
                cleaned_line += toupper(c);
            }
        }
        
        // Проверка длины
        if (cleaned_line.length() == 26) {
            // Проверка, что все символы A-Z
            bool valid = true;
            for (char c : cleaned_line) {
                if (c < 'A' || c > 'Z') {
                    valid = false;
                    break;
                }
            }
            
            if (valid) {
                rotors.push_back(cleaned_line);
                valid_rotors++;
            }
        }
    }
    
    file.close();
    
    cout << "Загружено роторов: " << valid_rotors << endl;
    
    if (valid_rotors == 0) {
        cout << "Не найдено валидных роторов!" << endl;
        return false;
    }
    
    return true;
}

// Оптимизированная функция дешифрования
string decrypt(const string& rotor1, const string& rotor2, const string& rotor3,
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

// Проверка на наличие флага
bool containsFlag(const string& text) {
    return text.find("SIBINTEK{") != string::npos;
}

void worker(int thread_id, int start_idx, int end_idx, int total_rotors) {
    auto start_time = chrono::high_resolution_clock::now();
    long long local_processed = 0;
    const int report_interval = 500000;
    
    for (int r1 = start_idx; r1 < end_idx && !found; r1++) {
        const string& rotor1 = rotors[r1];
        
        for (int r2 = 0; r2 < total_rotors && !found; r2++) {
            if (r2 == r1) continue;
            const string& rotor2 = rotors[r2];
            
            for (int r3 = 0; r3 < total_rotors && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                const string& rotor3 = rotors[r3];
                
                // Диапазоны позиций согласно hint.txt
                for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                    for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                        for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                            string plaintext = decrypt(rotor1, rotor2, rotor3, pos1, pos2, pos3);
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

int main() {
    auto program_start = chrono::high_resolution_clock::now();
    
    cout << "🚀 Загрузка роторов..." << endl;
    if (!loadRotors("dict.txt")) {
        return 1;
    }
    
    int total_rotors = rotors.size();
    int num_threads = thread::hardware_concurrency();
    
    // Используем все роторы для максимального покрытия
    int test_rotors = total_rotors;
    
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
        threads.emplace_back(worker, i, start, end, test_rotors);
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
        cout << "❌ Флаг не найден." << endl;
        cout << "Возможные причины:" << endl;
        cout << "1. Алгоритм шифрования может отличаться от предполагаемого" << endl;
        cout << "2. Флаг может иметь другой формат" << endl;
        cout << "3. Могут быть дополнительные компоненты в схеме шифрования" << endl;
    }
    
    return 0;
}
