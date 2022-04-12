#include <iostream>
#include <string>
#include <vector>

class BigInteger {
private:
    const int static number_ = 1e9;
    std::vector<long long> bigNumber_;
    int sign_;

    size_t length() const { return bigNumber_.size(); }

    void addition(const BigInteger &argument) {
        if (sign_ == 0) sign_ = argument.sign_;
        if (bigNumber_.size() < argument.bigNumber_.size())
            bigNumber_.resize(argument.bigNumber_.size());
        long long last = 0;
        for (size_t i = 0; i < bigNumber_.size(); ++i) {
            long long sum = bigNumber_[i] + last;
            if (i < argument.bigNumber_.size()) sum += argument.bigNumber_[i];
            bigNumber_[i] = sum % number_;
            last = sum / number_;
        }
        if (last != 0) bigNumber_.push_back(last);
    }

    void subtraction(const BigInteger &argument) {
        long long delta = 0;
        if (compare(argument) == false) {
            bool isZero = 1;
            for (size_t i = 0; i < bigNumber_.size(); ++i) {
                delta = bigNumber_[i];
                if (i < argument.bigNumber_.size())
                    delta = bigNumber_[i] - argument.bigNumber_[i];
                if (delta < 0) {
                    --bigNumber_[i + 1];
                    delta += number_;
                }
                bigNumber_[i] = delta;
                if (bigNumber_[i] != 0) isZero = 0;
            }
            if (isZero) sign_ = 0;

        } else {
            if (bigNumber_.size() < argument.bigNumber_.size())
                bigNumber_.resize(argument.bigNumber_.size());
            long long part = 0;
            if (sign_ == 0) sign_ = -1;
            sign_ = -sign_;
            for (size_t i = 0; i < bigNumber_.size(); ++i) {
                delta = argument.bigNumber_[i] - bigNumber_[i] - part;
                if (delta < 0) {
                    part = 1;
                    delta += number_;
                } else
                    part = 0;
                bigNumber_[i] = delta;
            }
        }
    }

    void twoPart() {
        bigNumber_[0] >>= 1;
        for (size_t index = 1; index < bigNumber_.size(); ++index) {
            if (bigNumber_[index] & 1) {
                bigNumber_[index - 1] += number_ / 2;
            }
            bigNumber_[index] >>= 1;
        }
        size_t i = bigNumber_.size() - 1;
        while (bigNumber_[i] == 0 && bigNumber_.size() > 1) {
            bigNumber_.pop_back();
            --i;
        }
        if (bigNumber_.size() == 1 && bigNumber_[0] == 0)sign_ = 0;

    }

    static void multiply(const BigInteger &withZeroes, BigInteger &result,
                         const BigInteger &argument, size_t zer) {
        long long multi;
        long long last = 0;
        result.bigNumber_.resize(argument.bigNumber_.size() + zer - 1, 0);
        for (size_t i = 0; i < argument.bigNumber_.size(); ++i) {
            multi = argument.bigNumber_[i] * withZeroes.bigNumber_[zer - 1];
            result.bigNumber_[i + zer - 1] = (multi + last) % number_;
            last = (multi + last) / number_;
        }
        result.sign_ = 1;
        if (last != 0) result.bigNumber_.push_back(last);
    }


public:
    BigInteger(long long x) {
        if (x < 0) {
            sign_ = -1;
            x = -x;
        } else {
            if (x > 0) sign_ = 1;
            if (x == 0) {
                sign_ = 0;
                bigNumber_.push_back(0);
                return;
            }
        }

        while (x != 0) {
            bigNumber_.push_back(x % number_);
            x /= number_;
        }
    }

    BigInteger() : BigInteger(0ll) {}

    BigInteger(const std::string &stringInput) {
        if (stringInput == "-0" || stringInput == "0") {
            sign_ = 0;
            bigNumber_.push_back(0);
            return;
        }
        size_t start;
        if (stringInput[0] == '-') {
            sign_ = -1;
            start = 1;
        } else {
            sign_ = 1;
            start = 0;
        }
        size_t newSize = ((stringInput.length() - start - 1) / 9) + 1;
        bigNumber_.resize(newSize);
        for (size_t i = 0; i < newSize - 1; ++i) {
            bigNumber_[i] =
                    std::stoll(stringInput.substr(stringInput.size() - 9 * (i + 1), 9));
        }
        if ((stringInput.length() - start) % 9 != 0) {
            bigNumber_.back() = std::stoll(
                    stringInput.substr(start, (stringInput.length() - start) % 9));
        } else
            bigNumber_.back() = std::stoll(stringInput.substr(start, 9));
    }

    int BigIntegerSign() const { return sign_; }


    void swap(const BigInteger &argument) {
        BigInteger copy = argument;
        std::swap(bigNumber_, copy.bigNumber_);
        std::swap(sign_, copy.sign_);
    }

    BigInteger &operator=(const BigInteger &argument) {
        swap(argument);
        return *this;
    }

    void invert() {
        sign_ *= -1;
    }

    const std::string toString() const {
        std::string result;
        std::string block;
        if (sign_ == -1) result.push_back('-');
        result += std::to_string(bigNumber_[length() - 1]);
        for (size_t i = length() - 1; i > 0; --i) {
            block = std::to_string(bigNumber_[i - 1]);
            size_t blockSize = block.size();
            while (blockSize < 9) {
                result += '0';
                ++blockSize;
            }
            result += block;
        }
        return result;
    }

    bool compare(const BigInteger &argument) const {
        if (length() < argument.length()) return true;
        if (length() > argument.length()) return false;
        for (size_t i = length(); i > 0; --i) {
            if (bigNumber_[i - 1] < argument.bigNumber_[i - 1]) return true;
            if (bigNumber_[i - 1] > argument.bigNumber_[i - 1]) return false;
        }
        return false;
    }

    explicit operator bool() const { return sign_; }

    BigInteger &operator+=(const BigInteger &argument);

    BigInteger &operator-=(const BigInteger &argument);

    BigInteger &operator*=(const BigInteger &argument);

    BigInteger &operator/=(const BigInteger &argument);

    BigInteger &operator%=(const BigInteger &argument);

    static void reduction(BigInteger &num, BigInteger &den);


    BigInteger operator-() const {
        BigInteger newBigInteger(*this);
        newBigInteger.sign_ = -sign_;
        return newBigInteger;
    }

    BigInteger &operator++() {
        *this += 1;
        return *this;
    }

    BigInteger operator++(int) {
        BigInteger newBigInteger = *this;
        ++(*this);
        return newBigInteger;
    }

    BigInteger &operator--() {
        *this -= 1;
        return *this;
    }

    BigInteger operator--(int) {
        BigInteger newBigInteger = *this;
        --(*this);
        return newBigInteger;
    }

    friend std::ostream &operator<<(std::ostream &Ostream,
                                    const BigInteger &argument);
};

std::istream &operator>>(std::istream &Istream, BigInteger &argument) {
    std::string inputString;
    Istream >> inputString;
    argument = BigInteger(inputString);
    return Istream;
}

std::ostream &operator<<(std::ostream &Ostream, const BigInteger &argument) {
    bool zero = false;
    if (argument.BigIntegerSign() == -1) Ostream << '-';
    if (argument.bigNumber_.size() == 1) {
        Ostream << argument.bigNumber_[0];
        return Ostream;
    }
    for (size_t i = argument.length(); i > 0; --i) {
        if (argument.bigNumber_[i - 1] == 0 && zero) Ostream << "000000000";
        if (argument.bigNumber_[i - 1] != 0) {
            long long gran = 1e8;
            if (zero) {
                while (argument.bigNumber_[i - 1] / gran == 0) {
                    Ostream << '0';
                    gran /= 10;
                }
            }
            zero = true;
            Ostream << argument.bigNumber_[i - 1];
        }
    }
    return Ostream;
}

BigInteger operator+(const BigInteger &lhs, const BigInteger &rhs) {
    BigInteger newBigInteger(lhs);
    newBigInteger += rhs;
    return newBigInteger;
}

BigInteger operator-(const BigInteger &lhs, const BigInteger &rhs) {
    BigInteger newBigInteger(lhs);
    newBigInteger -= rhs;
    return newBigInteger;
}

BigInteger operator*(const BigInteger &lhs, const BigInteger &rhs) {
    BigInteger newBigInteger(lhs);
    newBigInteger *= rhs;
    return newBigInteger;
}

bool operator<(const BigInteger &lhs, const BigInteger &rhs) {
    if (lhs.BigIntegerSign() < rhs.BigIntegerSign()) return true;
    if (lhs.BigIntegerSign() > rhs.BigIntegerSign()) return false;
    if (lhs.BigIntegerSign() == 0) return false;
    bool comparing = lhs.compare(rhs);
    if (lhs.BigIntegerSign() < 0) {
        return !comparing;
    }
    return comparing;
}

bool operator>(const BigInteger &lhs, const BigInteger &rhs) {
    return rhs < lhs;
}

bool operator==(const BigInteger &lhs, const BigInteger &rhs) {
    return !(lhs < rhs) && !(lhs > rhs);
}

bool operator<=(const BigInteger &lhs, const BigInteger &rhs) {
    return !(lhs > rhs);
}

bool operator>=(const BigInteger &lhs, const BigInteger &rhs) {
    return !(lhs < rhs);
}

bool operator!=(const BigInteger &lhs, const BigInteger &rhs) {
    return !(lhs == rhs);
}

BigInteger &BigInteger::operator+=(const BigInteger &argument) {
    if (this == &argument) {
        BigInteger copy = argument;
        return *this += copy;
    }
    if (sign_ * argument.sign_ >= 0)
        addition(argument);
    else {
        subtraction(argument);
        size_t i = bigNumber_.size() - 1;
        while (bigNumber_[i] == 0 && bigNumber_.size() > 1) {
            bigNumber_.pop_back();
            --i;
        }
    }
    return *this;
}

BigInteger &BigInteger::operator-=(const BigInteger &argument) {
    if (this == &argument) {
        *this = 0;
        return *this;
    }
    if (sign_ * argument.sign_ >= 0)
        subtraction(argument);
    else
        addition(argument);
    size_t i = bigNumber_.size() - 1;
    while (bigNumber_[i] == 0 && bigNumber_.size() > 1) {
        bigNumber_.pop_back();
        --i;
    }
    return *this;
}

BigInteger &BigInteger::operator*=(const BigInteger &argument) {
    if (this == &argument) {
        BigInteger copy = argument;
        return *this *= copy;
    }
    if (argument.sign_ == 0) {
        *this = BigInteger(0ll);
        return *this;
    }
    BigInteger newBigInteger;
    long long next = 0;
    long long temp;
    long long dop;
    newBigInteger.sign_ = sign_ * argument.sign_;
    newBigInteger.bigNumber_.resize(bigNumber_.size() + argument.bigNumber_.size() +
                                    1);
    for (size_t i = 0; i < bigNumber_.size(); ++i) {
        for (size_t k = 0; k < argument.bigNumber_.size() || next; ++k) {
            dop = k < argument.bigNumber_.size() ? argument.bigNumber_[k] : 0;
            temp = newBigInteger.bigNumber_[i + k] + bigNumber_[i] * dop + next;
            newBigInteger.bigNumber_[i + k] = temp % number_;
            next = temp / number_;
        }
    }
    if (next != 0) newBigInteger.bigNumber_.push_back(next);

    *this = newBigInteger;

    size_t i = bigNumber_.size() - 1;
    while (bigNumber_[i] == 0 && bigNumber_.size() > 1) {
        bigNumber_.pop_back();
        --i;
    }

    return *this;
}

BigInteger &BigInteger::operator/=(const BigInteger &argument) {
    if (this == &argument) {
        BigInteger copy = argument;
        return *this /= copy;
    }
    BigInteger answer;
    if (sign_ == 0) {
        answer = 0;
        *this = answer;
        return *this;
    }
    if (compare(argument) == true) {
        answer = 0;
        *this = answer;
        return *this;
    }
    BigInteger newBigInteger;
    size_t zer = bigNumber_.size() - argument.bigNumber_.size() + 1;
    answer.bigNumber_.resize(zer);
    newBigInteger.bigNumber_.resize(zer, 0);
    answer.sign_ = sign_ * argument.sign_;
    sign_ = 1;

    for (size_t i = bigNumber_.size() - argument.bigNumber_.size() + 1; i > 0; --i) {
        long long left = -1;
        long long right = 1e9;
        BigInteger result;
        while (left < right - 1) {
            long long median = (left + right) / 2;
            newBigInteger.bigNumber_[i - 1] = median;
            multiply(newBigInteger, result, argument, zer);
            if (result <= *this) left = median;
            if (result > *this) right = median;
        }
        newBigInteger.bigNumber_[i - 1] = left;
        multiply(newBigInteger, result, argument, zer);
        // std::cout << "this: " << *this << "  result:" << result << std::endl;
        *this -= result;
        --zer;
        answer.bigNumber_[i - 1] = left;
        newBigInteger.bigNumber_[i - 1] = 0;
        if (*this == 0) break;
    }
    *this = answer;
    size_t i = bigNumber_.size() - 1;
    while (bigNumber_[i] == 0 && bigNumber_.size() > 1) {
        bigNumber_.pop_back();
        --i;
    }
    return *this;
}

BigInteger operator/(const BigInteger &lhs, const BigInteger &rhs) {
    BigInteger newBigInteger(lhs);
    newBigInteger /= rhs;
    return newBigInteger;
}

BigInteger &BigInteger::operator%=(const BigInteger &argument) {
    BigInteger a = ((*this / argument) * argument);
    *this -= a;
    return *this;
}

BigInteger operator%(const BigInteger &lhs, const BigInteger &rhs) {
    BigInteger newBigInteger(lhs);
    newBigInteger %= rhs;
    return newBigInteger;
}


void BigInteger::reduction(BigInteger &num, BigInteger &den) {
    while (num.bigNumber_[0] % 2 == 0 && den.bigNumber_[0] % 2 == 0) {
        num.twoPart();
        den.twoPart();
    }
    BigInteger a = num;
    BigInteger b = den;
    while (a != 0 && b != 0) {
        while (a.bigNumber_[0] % 2 == 0) a.twoPart();
        while (b.bigNumber_[0] % 2 == 0) b.twoPart();
        if (a > b) {
            a -= b;
            a.twoPart();
        } else {
            b -= a;
            b.twoPart();
        }

    }
    if (a == 0)std::swap(a, b);
    num /= a;
    den /= a;
}

class Rational {
private:
    BigInteger numerator_;
    BigInteger denominator_;
    int sign_;

    void addition(const Rational &argument) {
        numerator_ =
                numerator_ * argument.denominator_ + denominator_ * argument.numerator_;
        denominator_ *= argument.denominator_;
    }

    void subtraction(const Rational &argument) {
        int a = compare(argument);
        BigInteger up = numerator_ * argument.denominator_;
        BigInteger down = denominator_ * argument.numerator_;
        if (a == false) {
            numerator_ = up - down;
            denominator_ *= argument.denominator_;
        }
        if (a == true) {
            sign_ = -sign_;
            numerator_ = down - up;
            denominator_ *= argument.denominator_;
        }
    }

public:
    Rational(int x) {
        if (x == 0) {
            sign_ = 0;
        }
        if (x > 0) {
            sign_ = 1;
        }
        if (x < 0) {
            sign_ = -1;
            x = -x;
        }
        numerator_ = x;
        denominator_ = 1;
    }


    Rational(const BigInteger &up, const BigInteger &down = 1) : numerator_(up), denominator_(down) {
        sign_ = up.BigIntegerSign() * down.BigIntegerSign();
        if (up.BigIntegerSign() == -1) numerator_.invert();
        if (down.BigIntegerSign() == -1) denominator_.invert();
        up.reduction(numerator_, denominator_);
    }


    Rational() : Rational(0) {}

    int BigIntegerSign() const { return sign_; }

    Rational operator-() const {
        Rational newRational(*this);
        newRational.sign_ = -sign_;
        return newRational;
    }

    const std::string toString() const {
        std::string result;
        if (sign_ == -1) result.push_back('-');
        result += numerator_.toString();
        if (denominator_ != 1) {
            result += '/';
            result += denominator_.toString();
        }
        return result;
    }

    bool compare(const Rational &argument) const {
        BigInteger up = numerator_ * argument.denominator_;
        BigInteger down = denominator_ * argument.numerator_;
        return up.compare(down);
    }

    Rational &operator+=(const Rational &argument) {
        if (this == &argument) {
            Rational copy = argument;
            return *this += copy;
        }
        if (sign_ * argument.sign_ >= 0) {
            addition(argument);
            if (sign_ == 0) sign_ = argument.sign_;

        } else {
            subtraction(argument);
        }
        numerator_.reduction(numerator_, denominator_);
        return *this;
    }

    Rational &operator-=(const Rational &argument) {
        if (this == &argument) {
            Rational copy = argument;
            return *this -= copy;
        }
        if (sign_ * argument.sign_ >= 0) {
            subtraction(argument);
            if (sign_ == 0) sign_ = -argument.sign_;

        } else {
            addition(argument);
        }
        numerator_.reduction(numerator_, denominator_);
        return *this;
    }

    Rational &operator*=(const Rational &argument) {
        if (this == &argument) {
            Rational copy = argument;
            return *this *= copy;
        }
        sign_ *= argument.sign_;
        numerator_ *= argument.numerator_;
        denominator_ *= argument.denominator_;
        numerator_.reduction(numerator_, denominator_);
        return *this;
    }

    Rational &operator/=(const Rational &argument) {
        if (this == &argument) {
            Rational copy = argument;
            return *this /= copy;
        }
        sign_ *= argument.sign_;
        numerator_ *= argument.denominator_;
        denominator_ *= argument.numerator_;
        numerator_.reduction(numerator_, denominator_);
        return *this;
    }

    std::string asDecimal(size_t precision) const {
        Rational newRational = *this;
        std::string result;
        std::string dopResult;
        if (sign_ == -1) result += '-';
        BigInteger push = 1e9;
        BigInteger pushBigInteger;
        pushBigInteger = newRational.numerator_ / newRational.denominator_;
        result += pushBigInteger.toString();
        result += '.';
        newRational.numerator_ -= pushBigInteger * denominator_;
        newRational.numerator_ *= push;
        while (precision > 0) {
            pushBigInteger = newRational.numerator_ / newRational.denominator_;
            newRational.numerator_ -= pushBigInteger * denominator_;
            newRational.numerator_ *= push;
            if (precision > pushBigInteger.toString().size()) {
                std::string part = pushBigInteger.toString();
                size_t n = part.length();
                while (n < 9) {
                    result += '0';
                    ++n;
                }
                result += pushBigInteger.toString();
                precision -= 9;
            } else {
                result += pushBigInteger.toString().substr(0, precision);
                break;
            }
        }
        return result;
    }

    explicit operator double() { return std::stod(asDecimal(30)); }

    friend std::ostream &operator<<(std::ostream &Ostream,
                                    const Rational &argument);

};

std::istream &operator>>(std::istream &Istream, Rational &argument) {
    BigInteger inputBiginteger;
    Istream >> inputBiginteger;
    argument = Rational(inputBiginteger);
    return Istream;
}

std::ostream &operator<<(std::ostream &Ostream, const Rational &argument) {
    if (argument.sign_ == -1) Ostream << '-';
    if (argument.denominator_ == 1)
        Ostream << argument.numerator_;
    else
        Ostream << argument.numerator_ << "/" << argument.denominator_;
    return Ostream;
}

bool operator==(const Rational &lhs, const Rational &rhs) {
    if ((lhs.BigIntegerSign() == rhs.BigIntegerSign()) && (lhs.compare(rhs) == 0))
        return true;
    else
        return false;
}

bool operator<(const Rational &lhs, const Rational &rhs) {
    if (lhs.BigIntegerSign() < rhs.BigIntegerSign()) return true;
    if (lhs.BigIntegerSign() > rhs.BigIntegerSign()) return false;
    if (lhs.BigIntegerSign() == 0) return false;
    return ((lhs.BigIntegerSign() == lhs.compare(rhs)));
}

bool operator>(const Rational &lhs, const Rational &rhs) {
    return rhs < lhs;
}

bool operator<=(const Rational &lhs, const Rational &rhs) {
    return !(lhs > rhs);
}

bool operator>=(const Rational &lhs, const Rational &rhs) {
    return !(lhs < rhs);
}

bool operator!=(const Rational &lhs, const Rational &rhs) {
    return !(lhs == rhs);
}

Rational operator+(const Rational &lhs, const Rational &rhs) {
    Rational newRational(lhs);
    newRational += rhs;
    return newRational;
}

Rational operator-(const Rational &lhs, const Rational &rhs) {
    Rational newRational(lhs);
    newRational -= rhs;
    return newRational;
}

Rational operator*(const Rational &lhs, const Rational &rhs) {
    Rational newRational(lhs);
    newRational *= rhs;
    return newRational;
}

Rational operator/(const Rational &lhs, const Rational &rhs) {
    Rational newRational(lhs);
    newRational /= rhs;
    return newRational;
}


