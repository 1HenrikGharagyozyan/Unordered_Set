#include <iostream>
#include <unordered_set>
#include "Unordered_Set.h"

int main() 
{
    // Тест 1: Инициализация через initializer_list
    Unordered_Set<std::string> fruits = { "apple", "banana", "cherry" };

    std::cout << "Initial set:\n";
    for (const auto& fruit : fruits) 
        std::cout << fruit << "\n";

    // Тест 2: insert и emplace
    fruits.insert("mango");
    fruits.emplace("banana"); // не должен добавиться повторно

    std::cout << "\nAfter insert/emplace:\n";
    for (const auto& fruit : fruits) 
        std::cout << fruit << "\n";

    // Тест 3: find и count
    if (fruits.find("banana") != fruits.end())
        std::cout << "\nFound banana!\n";

    std::cout << "Count of apple: " << fruits.count("apple") << "\n";
    std::cout << "Count of pear: " << fruits.count("pear") << "\n";

    // Тест 4: erase
    fruits.erase("apple");
    std::cout << "\nAfter erase(apple):\n";
    for (const auto& fruit : fruits)
        std::cout << fruit << "\n";

    // Тест 5: empty, size
    std::cout << "\nIs empty? " << (fruits.empty() ? "yes" : "no") << "\n";
    std::cout << "Size: " << fruits.size() << "\n";

    return 0;
}
