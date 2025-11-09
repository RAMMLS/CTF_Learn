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
#include <set>

using namespace std;

atomic<bool> found(false);
mutex cout_mutex;

vector<vector<int>> rotors;
string ciphertext;
vector<int> ciphertext_num;

// Частоты букв в английском языке (в порядке A-Z)
const vector<double> english_freq = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015, // A-G
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749, // H-N
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758, // O-U
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074                   // V-Z
};

// Частые английские слова для проверки
const vector<string> common_english_words = {
    "THE", "AND", "ING", "HER", "HAT", "HIS", "THA", "ERE", "FOR", "ENT",
    "ION", "TER", "WAS", "YOU", "ITH", "VER", "ALL", "WIT", "THI", "TIO"
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

// Расчет хи-квадрат для соответствия английскому языку
double chiSquaredTest(const string& text) {
    vector<int> counts(26, 0);
    int total_letters = 0;
    
    for (char c : text) {
        if (c >= 'A' && c <= 'Z') {
            counts[c - 'A']++;
            total_letters++;
        }
    }
    
    if (total_letters == 0) return 1e9;
    
    double chi_squared = 0.0;
    for (int i = 0; i < 26; i++) {
        double expected = english_freq[i] * total_letters;
        double observed = counts[i];
        if (expected > 0) {
            chi_squared += pow(observed - expected, 2) / expected;
        }
    }
    
    return chi_squared;
}

// Проверка на наличие общих английских слов
int countEnglishWords(const string& text) {
    int count = 0;
    for (const string& word : common_english_words) {
        if (text.find(word) != string::npos) {
            count++;
        }
    }
    return count;
}

// Проверка на наличие шаблона флага
bool checkFlagPattern(const string& text) {
    // Ищем SIBINTEK{...}
    size_t start = text.find("SIBINTEK");
    if (start == string::npos) return false;
    
    // Проверяем, что после SIBINTEK идет {
    if (start + 8 >= text.length() || text[start + 8] != '{') return false;
    
    // Ищем закрывающую }
    size_t end = text.find('}', start + 9);
    if (end == string::npos) return false;
    
    // Проверяем, что внутри флага только допустимые символы
    string flag_content = text.substr(start + 9, end - start - 9);
    for (char c : flag_content) {
        if (!isalnum(c) && c != '_') {
            return false;
        }
    }
    
    return true;
}

// Проверка на осмысленный английский текст
bool isMeaningfulEnglish(const string& text, double chi_threshold = 150.0, int min_words = 3) {
    // Тест хи-квадрат
    double chi_squared = chiSquaredTest(text);
    if (chi_squared > chi_threshold) return false;
    
    // Проверка общих английских слов
    int word_count = countEnglishWords(text);
    if (word_count < min_words) return false;
    
    // Проверка соотношения гласных/согласных
    int vowels = 0, consonants = 0;
    for (char c : text) {
        if (c >= 'A' && c <= 'Z') {
            if (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
                vowels++;
            } else {
                consonants++;
            }
        }
    }
    
    if (vowels + consonants == 0) return false;
    double vowel_ratio = (double)vowels / (vowels + consonants);
    
    // В английском обычно около 40% гласных
    if (vowel_ratio < 0.3 || vowel_ratio > 0.5) return false;
    
    return true;
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
    
    // Основная проверка: шаблон флага
    if (checkFlagPattern(result)) {
        return true;
    }
    
    // Дополнительная проверка: осмысленный английский текст
    if (isMeaningfulEnglish(result)) {
        // Если текст осмысленный, выводим его для ручной проверки
        lock_guard<mutex> lock(cout_mutex);
        cout << "Возможный вариант (английский текст): " << result.substr(0, 100) << "..." << endl;
    }
    
    return false;
}

void worker(int start_r1, int end_r1, int total_rotors) {
    for (int r1 = start_r1; r1 < end_r1 && !found; r1++) {
        if (r1 % 10 == 0) {
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
                            string plaintext;
                            if (decryptAndCheck(rotors[r1], rotors[r2], rotors[r3], pos1, pos2, pos3, plaintext)) {
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
    cout << "Общее количество комбинаций: " 
         << total_rotors * (total_rotors-1) * (total_rotors-2) * 10 * 10 * 16 
         << endl;
    
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
        cout << "Флаг не найден. Возможно, нужно:" << endl;
        cout << "1. Увеличить количество проверяемых роторов" << endl;
        cout << "2. Расширить диапазоны стартовых позиций" << endl;
        cout << "3. Проверить другие возможные шаблоны флага" << endl;
    }
    
    return 0;
}
