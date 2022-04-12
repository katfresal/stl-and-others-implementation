#include <iostream>
#include <cstring>

class String {

public:

    String() : size_(0), bufferSize_(1), symbols_(nullptr) {};

    String(const char *n) : size_(strlen(n)), bufferSize_(size_), symbols_(new char[bufferSize_]) {
        memcpy(symbols_, n, size_);
    }

    String(size_t n, char symbol) : size_(n), bufferSize_(n), symbols_(new char[n]) {
        std::memset(symbols_, symbol, n);
    }

    String(const String &other)
            : size_(other.size_), bufferSize_(other.bufferSize_), symbols_(new char[bufferSize_]) {
        memcpy(symbols_, other.symbols_, size_);
    }

    size_t length() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void clear() {
        size_ = 0;
        bufferSize_ = 1;
        delete[] symbols_;
        symbols_ = nullptr;
    }

    void push_back(char symbol) {
        if (size_ + 1 >= bufferSize_) growBuffer();
        symbols_[size_++] = symbol;
    }

    char &front() {
        return symbols_[0];
    }

    const char &front() const {
        return symbols_[0];
    }

    char &back() {
        return symbols_[size_ - 1];
    }

    const char &back() const {
        return symbols_[size_ - 1];
    }

    void pop_back() {
        size_--;
    }

    String substr(size_t start, size_t counter) const {
        String sub_string;
        for (size_t i = start; i < start + counter; i++) {
            sub_string.push_back(symbols_[i]);
        }
        return sub_string;
    }

    size_t find(const String &substr) const {
        for (size_t i = 0; i < length() - substr.length(); i++) {
            bool flag = true;
            for (size_t j = 0; j < substr.length(); j++) {
                if (symbols_[i + j] != substr[j]) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                return i;
            }
        }
        return length();
    }

    size_t rfind(const String &substr) const {
        for (size_t i = 0; i < length() - substr.length(); i++) {
            bool flag = 1;
            for (size_t j = 0; j < substr.length(); j++) {
                if (symbols_[length() - i - j - 1] != substr[substr.length() - j - 1]) {
                    flag = 0;
                    break;
                }
            }
            if (flag) {
                return length() - substr.length() - i;
            }
        }
        return length();
    }

    char &operator[](size_t i) {
        return symbols_[i];
    }

    char operator[](size_t i) const {
        return symbols_[i];
    }

    String &operator+=(const String &other) {
        size_t len = other.length();
        for (size_t i = 0; i < len; i++) {
            push_back(other[i]);
        }
        return *this;
    }
    String &operator+=(const char &symbol) {
        push_back(symbol);
        return *this;
    }


    void swap(String &other) {
        std::swap(symbols_, other.symbols_);
        std::swap(size_, other.size_);
        std::swap(bufferSize_, other.bufferSize_);
    }

    bool operator==(const String &other) const{
        if (other.length() != size_) return false;
        for (size_t i =0; i< size_; i++){
            if (symbols_[i] != other.symbols_[i]) return false;
        }
        return true;
    }

    String &operator=(const String &other) {
        if (!(other == *this)){
            String copy = other;
            swap(copy);
        }
        return *this;
    }

    String &operator=(const char *symbol) {
        clear();
        size_ = std::strlen(symbol);
        symbols_ = new char[size_ + 1];
        bufferSize_ = size_ + 1;
        memcpy(symbols_, symbol, size_);
        return *this;
    }





    ~String() {
        clear();
    }

private:
    size_t size_;
    size_t bufferSize_;
    char *symbols_;

    void growBuffer() {
        char *newSymbols = new char[bufferSize_ * 2];
        bufferSize_ *= 2;
        std::memcpy(newSymbols, symbols_, size_);
        delete[] symbols_;
        symbols_ = newSymbols;
    }
};
String operator+(const String &lhs, const String &rhs) {
    String newString(lhs);
    newString += rhs;
    return newString;
}

String operator+(const String &other, const char &symbol) {
    String newString(other);
    newString += symbol;
    return newString;
}

String operator+(const char symbol, const String &other) {
    String newString(1, symbol);
    newString += other;
    return newString;
}

std::ostream &operator<<(std::ostream &Ostream, const String &string) {
    for (size_t i = 0; i < string.length(); i++) {
        Ostream << string[i];
    }
    return Ostream;
}
std::istream &operator>>(std::istream &Istream, String &string) {
    string.clear();
    char input;
    while (Istream.get(input)) {
        if (input == '\n') break;

        string.push_back(input);
    }

    return Istream;
}
