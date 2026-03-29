#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>
#include <sstream>

#include <boost/thread.hpp>  

void processFile(const std::string& filename,
                  std::atomic<int>& counter,
                  std::mutex& coutMutex) {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        counter++;  
        return;
    }

    std::string line;
    int wordCount = 0;
    int charCount = 0;

    while (std::getline(inputFile, line)) {
        charCount += line.length() + 1; 

        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            wordCount++;
        }
    }

    inputFile.close();

    std::string outputFilename = "processed_" + filename;
    std::ofstream outputFile(outputFilename);

    if (outputFile.is_open()) {
        outputFile << "Результаты обработки файла: " << filename << std::endl;
        outputFile << "----------------------------------------" << std::endl;
        outputFile << "Количество символов: " << charCount << std::endl;
        outputFile << "Количество слов: " << wordCount << std::endl;
        outputFile.close();
    } else {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Ошибка: не удалось создать выходной файл " << outputFilename << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Обработан файл: " << filename << std::endl;
        std::cout << "  - Символов: " << charCount << std::endl;
        std::cout << "  - Слов: " << wordCount << std::endl;
        std::cout << "  - Результат сохранён в: " << outputFilename << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    counter++;
}

int main() {
    std::vector<std::string> files = {
        "data1.txt",
        "data2.txt",
        "data3.txt"
    };
    std::atomic<int> processedCount{0};
    std::mutex coutMutex;
    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<boost::thread> threads;
    for (const auto& filename : files) {
        threads.emplace_back(
            boost::bind(&processFile, filename,
                       boost::ref(processedCount),
                       boost::ref(coutMutex))
        );
    }

    for (auto& t : threads) {
        t.join();  
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "\n========== ИТОГОВАЯ СТАТИСТИКА ==========" << std::endl;
    std::cout << "Обработано файлов: " << processedCount.load() << " из " << files.size() << std::endl;
    std::cout << "Общее время выполнения: " << duration.count() << " мс" << std::endl;
    std::cout << "=========================================" << std::endl;
    return 0;
}
