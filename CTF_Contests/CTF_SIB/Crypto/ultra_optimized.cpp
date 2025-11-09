#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <filesystem>

using namespace std;

atomic<bool> found(false);
atomic<int> total_processed(0);
mutex cout_mutex;

vector<string> rotors;
string ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG";

// Схема подключения по описанию
const int CONSONANTS_COUNT = 20;
const int VOWELS_COUNT = 6;
const string consonants = "BCDFGHJKLMNPQRSTVWXZ";
const string vowels = "AEIOUY";

// Функция для проверки существования файла
bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

// Функция для вывода списка файлов в текущей директории
void listFilesInDirectory() {
    cout << "Файлы в текущей директории:" << endl;
    system("ls -la *.txt 2>/dev/null || dir *.txt 2>/dev/null || echo 'Не удалось получить список файлов'");
}

// Улучшенная функция загрузки роторов
bool loadRotorsImproved(const string& filename) {
    vector<string> possible_files = {
        "dict.txt", "dict (1).txt", "dict(1).txt", 
        "./dict.txt", "../dict.txt", "../../dict.txt",
        "dict", "dictionary.txt", "rotors.txt"
    };
    
    string actual_file;
    ifstream file;
    
    // Сначала проверяем существование файлов
    cout << "Поиск файла с роторами..." << endl;
    for (const auto& fname : possible_files) {
        if (fileExists(fname)) {
            cout << "Найден файл: " << fname << endl;
            actual_file = fname;
            file.open(fname);
            break;
        }
    }
    
    if (!file.is_open()) {
        cout << "Файл с роторами не найден!" << endl;
        listFilesInDirectory();
        return false;
    }
    
    cout << "Чтение файла: " << actual_file << endl;
    
    string line;
    int line_num = 0;
    int loaded = 0;
    
    while (getline(file, line)) {
        line_num++;
        
        // Удаляем все не-буквенные символы
        string cleaned_line;
        for (char c : line) {
            if (isalpha(c)) {
                cleaned_line += toupper(c);
            }
        }
        
        // Проверяем длину
        if (cleaned_line.length() == 26) {
            rotors.push_back(cleaned_line);
            loaded++;
            
            // Выводим первый ротор для проверки
            if (loaded == 1) {
                cout << "Первый ротор: " << cleaned_line << endl;
            }
        } else if (cleaned_line.length() > 0) {
            cout << "Пропущена строка " << line_num << ": неверная длина " << cleaned_line.length() << endl;
        }
        
        // Ограничиваем количество загружаемых роторов для теста
        if (loaded >= 100) {
            cout << "Ограничение: загружено 100 роторов (для теста)" << endl;
            break;
        }
    }
    
    file.close();
    
    if (loaded == 0) {
        cout << "Не загружено ни одного валидного ротора!" << endl;
        cout << "Проверьте формат файла. Каждая строка должна содержать 26 букв." << endl;
        return false;
    }
    
    cout << "Успешно загружено " << loaded << " роторов" << endl;
    return true;
}

// Создание тестовых роторов если файл не найден
void createTestRotors() {
    cout << "Создание тестовых роторов..." << endl;
    
    // Базовый ротор - перестановка алфавита
    string base_rotor = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    for (int i = 0; i < 50; i++) {
        // Слегка перемешиваем для разнообразия
        if (i > 0) {
            next_permutation(base_rotor.begin(), base_rotor.end());
        }
        rotors.push_back(base_rotor);
    }
    
    cout << "Создано " << rotors.size() << " тестовых роторов" << endl;
    cout << "Пример: " << rotors[0] << endl;
}

// Быстрая функция дешифрования согласно схеме
string decryptWithScheme(const string& rotor1, const string& rotor2, const string& rotor3, 
                        int pos1, int pos2, int pos3) {
    string result;
    result.reserve(ciphertext.length());
    
    int p1 = pos1, p2 = pos2, p3 = pos3;
    
    for (char c : ciphertext) {
        if (!isalpha(c)) {
            result += c;
            continue;
        }
        
        char current_char = toupper(c);
        char processed_char = current_char;
        
        // Определяем тип символа (согласная или гласная)
        bool is_consonant = (consonants.find(current_char) != string::npos);
        
        if (is_consonant) {
            // Обработка согласных через 20-позиционные переключатели
            int idx = consonants.find(current_char);
            
            // Обратный проход через три переключателя
            idx = (idx + p3) % CONSONANTS_COUNT;
            char c3 = rotor3[idx];
            size_t pos_in_cons = consonants.find(c3);
            if (pos_in_cons != string::npos) {
                idx = pos_in_cons;
            }
            idx = (idx - p3 + CONSONANTS_COUNT) % CONSONANTS_COUNT;
            
            idx = (idx + p2) % CONSONANTS_COUNT;
            char c2 = rotor2[idx];
            pos_in_cons = consonants.find(c2);
            if (pos_in_cons != string::npos) {
                idx = pos_in_cons;
            }
            idx = (idx - p2 + CONSONANTS_COUNT) % CONSONANTS_COUNT;
            
            idx = (idx + p1) % CONSONANTS_COUNT;
            char c1 = rotor1[idx];
            pos_in_cons = consonants.find(c1);
            if (pos_in_cons != string::npos) {
                idx = pos_in_cons;
            }
            idx = (idx - p1 + CONSONANTS_COUNT) % CONSONANTS_COUNT;
            
            processed_char = consonants[idx];
            
            // Вращение роторов ТОЛЬКО для согласных символов
            p1 = (p1 + 1) % CONSONANTS_COUNT;
            if (p1 == 0) {
                p2 = (p2 + 1) % CONSONANTS_COUNT;
                if (p2 == 0) {
                    p3 = (p3 + 1) % CONSONANTS_COUNT;
                }
            }
        }
        // Гласные пока оставляем без изменений для упрощения
        
        result += processed_char;
    }
    
    return result;
}

// Ультра-быстрая проверка флага
bool quickCheckFlag(const string& text) {
    if (text.length() < 50) return false;
    return text.find("SIBINTEK{") != string::npos;
}

void optimizedWorker(int start_r1, int end_r1) {
    const int TOTAL_ROTORS = rotors.size();
    int local_processed = 0;
    
    for (int r1 = start_r1; r1 < end_r1 && !found; r1++) {
        // Используем только первые 20 символов ротора для согласных
        string rotor1_cons = rotors[r1].substr(0, CONSONANTS_COUNT);
        
        for (int r2 = 0; r2 < TOTAL_ROTORS && !found; r2++) {
            if (r2 == r1) continue;
            string rotor2_cons = rotors[r2].substr(0, CONSONANTS_COUNT);
            
            for (int r3 = 0; r3 < TOTAL_ROTORS && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                string rotor3_cons = rotors[r3].substr(0, CONSONANTS_COUNT);
                
                // Только ключевые позиции из hint.txt
                for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                    for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                        for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                            local_processed++;
                            total_processed++;
                            
                            string plaintext = decryptWithScheme(
                                rotor1_cons, rotor2_cons, rotor3_cons, 
                                pos1, pos2, pos3
                            );
                            
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
                            
                            // Быстрый прогресс для первых комбинаций
                            if (local_processed % 1000 == 0 && local_processed < 10000) {
                                lock_guard<mutex> lock(cout_mutex);
                                cout << "[Поток] Проверено " << local_processed << " комбинаций, текущий ротор R1: " << r1 << endl;
                            }
                        }
                    }
                }
            }
        }
        
        // Прогресс каждые 2 ротора
        if ((r1 - start_r1) % 2 == 0) {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Проверено роторов R1: " << (r1 - start_r1) << "/" << (end_r1 - start_r1) 
                 << ", комбинаций: " << local_processed << endl;
        }
    }
}

int main() {
    auto start_time = chrono::high_resolution_clock::now();
    
    cout << "=== Purple Machine Brute Force ===" << endl;
    cout << "Загрузка роторов..." << endl;
    
    if (!loadRotorsImproved("dict.txt")) {
        cout << "Создаем тестовые роторы..." << endl;
        createTestRotors();
    }
    
    // ОГРАНИЧИВАЕМ количество роторов для быстрого теста
    int TEST_ROTORS = min(15, (int)rotors.size());
    cout << "\nНастройки брутфорса:" << endl;
    cout << "Тестируем роторов: " << TEST_ROTORS << endl;
    
    // Расчет комбинаций
    long long combinations = (long long)TEST_ROTORS * (TEST_ROTORS-1) * (TEST_ROTORS-2) * 10 * 10 * 16;
    cout << "Примерное количество комбинаций: " << combinations << endl;
    cout << "Ожидаемое время: ~" << combinations / 5000 / 60 << " минут при 5к комб/с" << endl;
    
    int num_threads = thread::hardware_concurrency();
    cout << "Используем " << num_threads << " потоков" << endl;
    
    vector<thread> threads;
    int rotors_per_thread = TEST_ROTORS / num_threads;
    
    if (rotors_per_thread == 0) {
        rotors_per_thread = 1;
        num_threads = min(num_threads, TEST_ROTORS);
    }
    
    cout << "Запуск потоков..." << endl;
    for (int i = 0; i < num_threads; i++) {
        int start = i * rotors_per_thread;
        int end = (i == num_threads - 1) ? TEST_ROTORS : (i + 1) * rotors_per_thread;
        if (start < TEST_ROTORS) {
            cout << "Поток " << i << ": роторы " << start << " - " << end-1 << endl;
            threads.emplace_back(optimizedWorker, start, end);
        }
    }
    
    // Мониторинг прогресса
    thread progress_thread([start_time]() {
        int last_processed = 0;
        auto last_time = start_time;
        
        while (!found) {
            this_thread::sleep_for(chrono::seconds(15));
            
            auto now = chrono::high_resolution_clock::now();
            int current_processed = total_processed;
            int delta = current_processed - last_processed;
            auto delta_time = chrono::duration_cast<chrono::seconds>(now - last_time);
            auto total_time = chrono::duration_cast<chrono::seconds>(now - start_time);
            
            if (delta_time.count() > 0) {
                int speed = delta / delta_time.count();
                lock_guard<mutex> lock(cout_mutex);
                cout << "[ПРОГРЕСС] " << current_processed << " комб, " 
                     << speed << " комб/с, " << total_time.count() / 60 << "мин" << endl;
            }
            
            last_processed = current_processed;
            last_time = now;
        }
    });
    
    // Ожидаем завершения
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    found = true;
    if (progress_thread.joinable()) {
        progress_thread.join();
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    
    cout << "\n=== РЕЗУЛЬТАТ ===" << endl;
    cout << "Общее время: " << duration.count() / 60 << " минут" << endl;
    cout << "Всего комбинаций: " << total_processed << endl;
    
    if (!found) {
        cout << "Флаг не найден в тестовом диапазоне." << endl;
        cout << "Рекомендации:" << endl;
        cout << "1. Увеличить TEST_ROTORS в коде" << endl;
        cout << "2. Проверить наличие файла dict.txt" << endl;
        cout << "3. Уточнить алгоритм дешифрования" << endl;
    }
    
    return 0;
}
