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
mutex cout_mutex;

vector<vector<int>> rotors;
string ciphertext;
vector<int> ciphertext_num;

// Функция для преобразования строки в числовой массив
vector<int> stringToNum(const string& str) {
    vector<int> result;
    for (char c : str) {
        result.push_back(c - 'A');
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
bool decryptAndCheck(const vector<int>& rotor1, const vector<int>& rotor2, const vector<int>& rotor3, 
                    int pos1, int pos2, int pos3, string& result) {
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
    
    result = numToString(plaintext);
    
    // Проверяем наличие SIBINTEK
    return result.find("Sibintex{}") != string::npos;
}

void worker(int start_r1, int end_r1, int total_rotors) {
    for (int r1 = start_r1; r1 < end_r1 && !found; r1++) {
        for (int r2 = 0; r2 < total_rotors && !found; r2++) {
            if (r2 == r1) continue;
            for (int r3 = 0; r3 < total_rotors && !found; r3++) {
                if (r3 == r1 || r3 == r2) continue;
                
                for (int pos1 = 1; pos1 <= 10 && !found; pos1++) {
                    for (int pos2 = 1; pos2 <= 10 && !found; pos2++) {
                        for (int pos3 = 10; pos3 <= 25 && !found; pos3++) {
                            string plaintext;
                            if (decryptAndCheck(rotors[r1], rotors[r2], rotors[r3], pos1, pos2, pos3, plaintext)) {
                                found = true;
                                lock_guard<mutex> lock(cout_mutex);
                                cout << "НАЙДЕНО!" << endl;
                                cout << "Роторы: " << r1 << ", " << r2 << ", " << r3 << endl;
                                cout << "Позиции: " << pos1 << ", " << pos2 << ", " << pos3 << endl;
                                cout << "Текст: " << plaintext << endl;
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

int main() {
    // Чтение роторов из файла
    ifstream file("dict.txt");
    string line;
    
    cout << "Загрузка роторов..." << endl;
    while (getline(file, line)) {
        if (line.length() == 26) {
            vector<int> rotor(26);
            for (int i = 0; i < 26; i++) {
                rotor[i] = line[i] - 'A';
            }
            rotors.push_back(rotor);
        }
    }
    file.close();
    
    cout << "Загружено " << rotors.size() << " роторов" << endl;
    
    // Подготовка шифртекста
    ciphertext = "KTHAUSGKAIZFXYTMNIMXJOXOMQMQSUKPLKUSQHAIHDEEQPFTNWNXXWJHOGHDQEXIHFQBOFEDJBQJHIJDENKKODYDEHNRRHUWJKDTGNAZXDNLJOUKSUADLLSMGSMBULPJREISOMTXWSYLDCQHDMKXQIUJNKQFPEQLPITOBYEADRSFPFKNUGQWMUGTBOXUOBMLLSPYSDTEUECAAGYKYZRONSBIJTXGNABGINVCXYSKAJAWBNHOGEBFNSGPKZVUYJAUYPDJHTBYAQQWTCKWCBWWXUHBJYEJRGAAPWDLWWIMIVUONBOAFNQESGWIOZXYRYRTYTSMUGRJNMATAQBKETEPQDERNAG";
    ciphertext_num = stringToNum(ciphertext);
    
    int total_rotors = rotors.size();
    int num_threads = thread::hardware_concurrency();
    cout << "Используем " << num_threads << " потоков" << endl;
    
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
    
    cout << "Время выполнения: " << duration.count() << " секунд" << endl;
    
    if (!found) {
        cout << "Флаг не найден" << endl;
    }
    
    return 0;
}
