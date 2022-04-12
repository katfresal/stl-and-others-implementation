#include <iostream>
#include <string>
#include <vector>
#include <cassert>

class BigInteger {
private:
    const int static number = 1e9;
    std::vector<long long> entrails;
    int sign;

    size_t length() const { return entrails.size(); }

public:
    BigInteger(long long x) {
        if (x < 0) {
            sign = -1;
            x = -x;
        } else {
            if (x > 0) sign = 1;
            if (x == 0) {
                sign = 0;
                entrails.push_back(0);
                return;
            }
        }

        while (x != 0) {
            entrails.push_back(x % number);
            x /= number;
        }
    }

    BigInteger() : BigInteger(0ll) {}

    BigInteger(const std::string &stringInput) {
        if (stringInput == "-0" || stringInput == "0") {
            sign = 0;
            entrails.push_back(0);
            return;
        }
        size_t start;
        if (stringInput[0] == '-') {
            sign = -1;
            start = 1;
        } else {
            sign = 1;
            start = 0;
        }
        size_t newSize = ((stringInput.length() - start - 1) / 9) + 1;
        entrails.resize(newSize);
        for (size_t i = 0; i < newSize - 1; ++i) {
            entrails[i] =
                    std::stoll(stringInput.substr(stringInput.size() - 9 * (i + 1), 9));
        }
        if ((stringInput.length() - start) % 9 != 0) {
            entrails.back() = std::stoll(
                    stringInput.substr(start, (stringInput.length() - start) % 9));
        } else
            entrails.back() = std::stoll(stringInput.substr(start, 9));
    }

    int signs() const { return sign; }

    int unsignedComparison(const BigInteger &argument) const {
        if (length() < argument.length()) return 1;
        if (length() > argument.length()) return -1;
        for (size_t i = length(); i > 0; --i) {
            if (entrails[i - 1] < argument.entrails[i - 1]) return 1;
            if (entrails[i - 1] > argument.entrails[i - 1]) return -1;
        }
        return 0;
    }

    void swap(BigInteger &argument) {
        std::swap(entrails, argument.entrails);
        std::swap(sign, argument.sign);
    }

    BigInteger &operator=(const BigInteger &argument) {
        BigInteger copy = argument;
        swap(copy);
        return *this;
    }

    void invert() {
        sign *= -1;
    }

    const std::string toString() const {
        std::string result;
        std::string x;
        if (sign == -1) result.push_back('-');
        result += std::to_string(entrails[length() - 1]);
        for (size_t i = length() - 1; i > 0; --i) {
            x = std::to_string(entrails[i - 1]);
            size_t sizeX = x.size();
            while (sizeX < 9) {
                result += '0';
                sizeX++;
            }
            result += x;
        }
        return result;
    }

    explicit operator bool() const { return sign; }

    void plus(const BigInteger &argument) {
        if (sign == 0) sign = argument.sign;
        if (entrails.size() < argument.entrails.size())
            entrails.resize(argument.entrails.size());
        long long last = 0;
        for (size_t i = 0; i < entrails.size(); ++i) {
            long long sum = entrails[i] + last;
            if (i < argument.entrails.size()) sum += argument.entrails[i];
            entrails[i] = sum % number;
            last = sum / number;
        }
        if (last != 0) entrails.push_back(last);
    }

    void minus(const BigInteger &argument) {
        if (unsignedComparison(argument) == 0) {
            *this = 0;
            return;
        }
        long long delta = 0;
        if (unsignedComparison(argument) != 1) {
            for (size_t i = 0; i < entrails.size(); ++i) {
                delta = entrails[i];
                if (i < argument.entrails.size())
                    delta = entrails[i] - argument.entrails[i];
                if (delta < 0) {
                    entrails[i + 1]--;
                    delta += number;
                }
                entrails[i] = delta;
            }

        } else {
            if (entrails.size() < argument.entrails.size())
                entrails.resize(argument.entrails.size());
            long long part = 0;
            if (sign == 0) sign = -1;
            sign = -sign;
            for (size_t i = 0; i < entrails.size(); ++i) {
                delta = argument.entrails[i] - entrails[i] - part;
                if (delta < 0) {
                    part = 1;
                    delta += number;
                } else
                    part = 0;
                entrails[i] = delta;
            }
        }
    }

    BigInteger &operator+=(const BigInteger &argument) {
        if (this == &argument) {
            BigInteger copy = argument;
            return *this += copy;
        }
        if (sign * argument.sign >= 0)
            plus(argument);
        else
            minus(argument);
        size_t i = entrails.size() - 1;
        while (entrails[i] == 0 && entrails.size() > 1) {
            entrails.pop_back();
            --i;
        }
        return *this;
    }

    BigInteger &operator-=(const BigInteger &argument) {
        if (this == &argument) {
            BigInteger copy = argument;
            return *this -= copy;
        }
        if (sign * argument.sign >= 0)
            minus(argument);
        else
            plus(argument);
        size_t i = entrails.size() - 1;
        while (entrails[i] == 0 && entrails.size() > 1) {
            entrails.pop_back();
            --i;
        }
        return *this;
    }

    BigInteger operator-() const {
        BigInteger newBigInteger(*this);
        newBigInteger.sign = -sign;
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

    BigInteger &operator*=(const BigInteger &argument) {
        if (this == &argument) {
            BigInteger copy = argument;
            return *this *= copy;
        }
        if (argument.sign == 0) {
            *this = BigInteger(0ll);
            return *this;
        }
        BigInteger newBigInteger;
        long long next = 0;
        long long temp;
        newBigInteger.sign = sign * argument.sign;
        newBigInteger.entrails.resize(entrails.size() + argument.entrails.size() +
                                      1);
        for (size_t i = 0; i < entrails.size(); ++i) {
            for (size_t k = 0; k < argument.entrails.size() || next; ++k) {
                temp = (newBigInteger.entrails[i + k] +
                        entrails[i] *
                        (k < argument.entrails.size() ? argument.entrails[k] : 0) +
                        next);
                newBigInteger.entrails[i + k] = temp % number;
                next = temp / number;
            }
        }
        if (next != 0) newBigInteger.entrails.push_back(next);

        *this = newBigInteger;

        size_t i = entrails.size() - 1;
        while (entrails[i] == 0 && entrails.size() > 1) {
            entrails.pop_back();
            --i;
        }

        return *this;
    }

    static void multiply(const BigInteger &withZeroes, BigInteger &result,
                         const BigInteger &argument, size_t zer) {
        long long multi;
        long long last = 0;
        result.entrails.resize(argument.entrails.size() + zer - 1, 0);
        for (size_t i = 0; i < argument.entrails.size(); ++i) {
            multi = argument.entrails[i] * withZeroes.entrails[zer - 1];
            result.entrails[i + zer - 1] = (multi + last) % number;
            last = (multi + last) / number;
        }
        result.sign = 1;
        if (last != 0) result.entrails.push_back(last);
    }

    void twoPart() {
        entrails[0] >>= 1;
        for (size_t index = 1; index < entrails.size(); ++index) {
            if (entrails[index] & 1) {
                entrails[index - 1] += number / 2;
            }
            entrails[index] >>= 1;
        }
        size_t i = entrails.size() - 1;
        while (entrails[i] == 0 && entrails.size() > 1) {
            entrails.pop_back();
            --i;
        }
        if (entrails.size() == 1 && entrails[0] == 0)sign = 0;

    }


    static void reduction(BigInteger &num, BigInteger &den);


    BigInteger &operator/=(const BigInteger &argument);

    BigInteger &operator%=(const BigInteger &argument);

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
    if (argument.signs() == -1) Ostream << '-';
    if (argument.entrails.size() == 1) {
        Ostream << argument.entrails[0];
        return Ostream;
    }
    for (size_t i = argument.length(); i > 0; i--) {
        if (argument.entrails[i - 1] == 0 && zero) Ostream << "000000000";
        if (argument.entrails[i - 1] != 0) {
            long long gran = 1e8;
            if (zero) {
                while (argument.entrails[i - 1] / gran == 0) {
                    Ostream << '0';
                    gran /= 10;
                }
            }
            zero = true;
            Ostream << argument.entrails[i - 1];
        }
    }
    return Ostream;
}

BigInteger operator+(const BigInteger &argument1, const BigInteger &argument2) {
    BigInteger newBigInteger(argument1);
    newBigInteger += argument2;
    return newBigInteger;
}

BigInteger operator-(const BigInteger &argument1, const BigInteger &argument2) {
    BigInteger newBigInteger(argument1);
    newBigInteger -= argument2;
    return newBigInteger;
}

BigInteger operator*(const BigInteger &argument1, const BigInteger &argument2) {
    BigInteger newBigInteger(argument1);
    newBigInteger *= argument2;
    // std::cerr << (argument1) << " " << "*" << " " << (argument2) << " =" <<
    // newBigInteger << std::endl;
    return newBigInteger;
}

bool operator==(const BigInteger &argument1, const BigInteger &argument2) {
    if ((argument1.unsignedComparison(argument2) == 0) &&
        (argument1.signs() == argument2.signs()))
        return true;
    else
        return false;
}

bool operator<(const BigInteger &argument1, const BigInteger &argument2) {
    if (argument1.signs() < argument2.signs()) return true;
    if (argument1.signs() > argument2.signs()) return false;
    if (argument1.signs() == 0) return false;

    return ((argument1.signs() == argument1.unsignedComparison(argument2)));
}

bool operator>(const BigInteger &argument1, const BigInteger &argument2) {
    return argument2 < argument1;
}

bool operator<=(const BigInteger &argument1, const BigInteger &argument2) {
    return !(argument1 > argument2);
}

bool operator>=(const BigInteger &argument1, const BigInteger &argument2) {
    return !(argument1 < argument2);
}

bool operator!=(const BigInteger &argument1, const BigInteger &argument2) {
    return !(argument1 == argument2);
}

BigInteger &BigInteger::operator/=(const BigInteger &argument) {
    if (this == &argument) {
        BigInteger copy = argument;
        return *this /= copy;
    }
    BigInteger answer;
    if (sign == 0) {
        answer = 0;
        *this = answer;
        return *this;
    }
    if (unsignedComparison(argument) == 1) {
        answer = 0;
        *this = answer;
        return *this;
    }
    BigInteger newBigInteger;
    size_t zer = entrails.size() - argument.entrails.size() + 1;
    answer.entrails.resize(zer);
    newBigInteger.entrails.resize(zer, 0);
    answer.sign = sign * argument.sign;
    sign = 1;

    for (size_t i = entrails.size() - argument.entrails.size() + 1; i > 0; --i) {
        //if (entrails[i-1] < argument){
        //  answer.entrails[i - 1] = 0;
        //entrails[i + argument.entrails.size() - 2] +=
        //}
        long long left = -1;
        long long right = 1e9;
        BigInteger result;
        while (left < right - 1) {
            long long median = (left + right) / 2;
            newBigInteger.entrails[i - 1] = median;
            multiply(newBigInteger, result, argument, zer);
            if (result <= *this) left = median;
            if (result > *this) right = median;
        }
        newBigInteger.entrails[i - 1] = left;
        multiply(newBigInteger, result, argument, zer);
        // std::cout << "this: " << *this << "  result:" << result << std::endl;
        *this -= result;
        --zer;
        answer.entrails[i - 1] = left;
        newBigInteger.entrails[i - 1] = 0;
        if (*this == 0) break;
    }
    *this = answer;
    size_t i = entrails.size() - 1;
    while (entrails[i] == 0 && entrails.size() > 1) {
        entrails.pop_back();
        --i;
    }
    return *this;
}

BigInteger operator/(const BigInteger &argument1, const BigInteger &argument2) {
    BigInteger newBigInteger(argument1);
    newBigInteger /= argument2;
    // std::cerr << argument1 << " " << "/" << " " << argument2 << " =" <<
    // newBigInteger << std::endl;
    return newBigInteger;
}

BigInteger &BigInteger::operator%=(const BigInteger &argument) {
    BigInteger a = ((*this / argument) * argument);
    *this -= a;
    return *this;
}

BigInteger operator%(const BigInteger &argument1, const BigInteger &argument2) {
    BigInteger newBigInteger(argument1);
    newBigInteger %= argument2;
    return newBigInteger;
}


void BigInteger::reduction(BigInteger &num, BigInteger &den) {
    while (num.entrails[0] % 2 == 0 && den.entrails[0] % 2 == 0) {
        num.twoPart();
        den.twoPart();
    }
    BigInteger a = num;
    BigInteger b = den;
    while (a != 0 && b != 0) {
        while (a.entrails[0] % 2 == 0) a.twoPart();
        while (b.entrails[0] % 2 == 0) b.twoPart();
        //std::cout << "into" << std::endl;
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
    BigInteger numerator;
    BigInteger denominator;
    int sign;

public:
    Rational(int x) {
        if (x == 0) {
            sign = 0;
        }
        if (x > 0) {
            sign = 1;
        }
        if (x < 0) {
            sign = -1;
            x = -x;
        }
        numerator = x;
        denominator = 1;
    }


    Rational(const BigInteger &up, const BigInteger &down = 1) : numerator(up), denominator(down) {
        sign = up.signs() * down.signs();
        if (up.signs() == -1) numerator.invert();
        if (down.signs() == -1) denominator.invert();
        up.reduction(numerator, denominator);
    }


    Rational() : Rational(0) {}

    int signs() const { return sign; }

    Rational operator-() const {
        Rational newRational(*this);
        newRational.sign = -sign;
        return newRational;
    }

    const std::string toString() const {
        //-
        std::string result;
        if (sign == -1) result.push_back('-');
        result += numerator.toString();
        if (denominator != 1) {
            result += '/';
            result += denominator.toString();
        }
        return result;
    }

    int unsignedComparison(const Rational &argument) const {
        BigInteger up = numerator * argument.denominator;
        BigInteger down = denominator * argument.numerator;
        return up.unsignedComparison(down);
    }

    void plus(const Rational &argument) {
        numerator =
                numerator * argument.denominator + denominator * argument.numerator;
        denominator *= argument.denominator;
    }

    void minus(const Rational &argument) {
        int a = unsignedComparison(argument);
        if (a == 0) {
            *this = 0;
            return;
        }
        BigInteger up = numerator * argument.denominator;
        BigInteger down = denominator * argument.numerator;
        if (a == -1) {
            numerator = up - down;
            denominator *= argument.denominator;
        }
        if (a == 1) {
            sign = -sign;
            numerator = down - up;
            denominator *= argument.denominator;
        }
    }

    Rational &operator+=(const Rational &argument) {
        if (this == &argument) {
            Rational copy = argument;
            return *this += copy;
        }
        if (sign * argument.sign >= 0) {
            plus(argument);
            if (sign == 0) sign = argument.sign;

        } else {
            minus(argument);
        }
        numerator.reduction(numerator, denominator);
        return *this;
    }

    Rational &operator-=(const Rational &argument) {
        if (this == &argument) {
            Rational copy = argument;
            return *this -= copy;
        }
        if (sign * argument.sign >= 0) {
            minus(argument);
            if (sign == 0) sign = -argument.sign;

        } else {
            plus(argument);
        }
        numerator.reduction(numerator, denominator);
        return *this;
    }

    Rational &operator*=(const Rational &argument) {
        if (this == &argument) {
            Rational copy = argument;
            return *this *= copy;
        }
        sign *= argument.sign;
        numerator *= argument.numerator;
        denominator *= argument.denominator;
        numerator.reduction(numerator, denominator);
        return *this;
    }

    Rational &operator/=(const Rational &argument) {
        if (this == &argument) {
            Rational copy = argument;
            return *this /= copy;
        }
        sign *= argument.sign;
        numerator *= argument.denominator;
        denominator *= argument.numerator;
        numerator.reduction(numerator, denominator);
        return *this;
    }

    std::string asDecimal(size_t precision) const {
        Rational newRational = *this;
        std::string result;
        std::string dopResult;
        if (sign == -1) result += '-';
        BigInteger push = 1e9;
        BigInteger pushBigInteger;
        pushBigInteger = newRational.numerator / newRational.denominator;
        result += pushBigInteger.toString();
        result += '.';
        ///bool op = 1;
        newRational.numerator -= pushBigInteger * denominator;
        newRational.numerator *= push;
        while (precision > 0) {
            pushBigInteger = newRational.numerator / newRational.denominator;
            newRational.numerator -= pushBigInteger * denominator;
            newRational.numerator *= push;
            if (precision > pushBigInteger.toString().size()) {
                if (pushBigInteger == 0) {
                    result += "000000000";
                    precision -= 9;
                } else {
                    std::string part = pushBigInteger.toString();
                    size_t n = part.length();
                    while (n < 9) {
                        result += '0';
                        n++;
                    }
                    result += pushBigInteger.toString();
                    precision -= pushBigInteger.toString().size();
                }

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
    if (argument.sign == -1) Ostream << '-';
    if (argument.denominator == 1)
        Ostream << argument.numerator;
    else
        Ostream << argument.numerator << "/" << argument.denominator;
    return Ostream;
}

bool operator==(const Rational &argument1, const Rational &argument2) {
    if ((argument1.signs() == argument2.signs()) && (argument1.unsignedComparison(argument2) == 0))
        return true;
    else
        return false;
}

bool operator<(const Rational &argument1, const Rational &argument2) {
    if (argument1.signs() < argument2.signs()) return true;
    if (argument1.signs() > argument2.signs()) return false;
    if (argument1.signs() == 0) return false;

    // std::cerr << (argument1) << " " << "<" << " " << (argument2) << " =" <<
    // ((argument1.signs() == argument1.unsignedComparison(argument2))) <<
    // std::endl;
    return ((argument1.signs() == argument1.unsignedComparison(argument2)));
}

bool operator>(const Rational &argument1, const Rational &argument2) {
    // std::cerr << (argument1) << " " << ">" << " " << (argument2) << " =" <<
    // (argument2 < argument1) << std::endl;
    return argument2 < argument1;
}

bool operator<=(const Rational &argument1, const Rational &argument2) {
    // std::cerr << (argument1) << " " << ">=" << " " << (argument2) << " =" <<
    // (!(argument1 > argument2)) << std::endl;
    return !(argument1 > argument2);
}

bool operator>=(const Rational &argument1, const Rational &argument2) {
    // std::cerr << (argument1) << " " << ">=" << " " << (argument2) << " =" <<
    // (!(argument1 < argument2)) << std::endl;
    return !(argument1 < argument2);
}

bool operator!=(const Rational &argument1, const Rational &argument2) {
    // std::cerr << (argument1) << " " << "!=" << " " << (argument2) << " =" <<
    // (!(argument1 == argument2)) << std::endl;

    return !(argument1 == argument2);
}

Rational operator+(const Rational &argument1, const Rational &argument2) {
    Rational newRational(argument1);
    newRational += argument2;

    // std::cerr << (argument1) << " " << "+" << " " << (argument2) << " =" <<
    // newRational << std::endl;
    return newRational;
}

Rational operator-(const Rational &argument1, const Rational &argument2) {
    Rational newRational(argument1);
    newRational -= argument2;

    // std::cerr << (argument1) << " " << "-" << " " << (argument2) << " =" <<
    // newRational << std::endl;
    return newRational;
}

Rational operator*(const Rational &argument1, const Rational &argument2) {
    Rational newRational(argument1);
    newRational *= argument2;

    // std::cerr << (argument1) << " " << "*" << " " << (argument2) << " =" <<
    // (newRational) << std::endl;
    return newRational;
}

Rational operator/(const Rational &argument1, const Rational &argument2) {
    Rational newRational(argument1);
    newRational /= argument2;

    // std::cerr << (argument1) << " " << "/" << " " << (argument2) << " =" <<
    // newRational << std::endl;
    return newRational;
}

template<size_t N, size_t I>
const bool is_prime = (I * I > N || N % I != 0) && is_prime<N, I + 1>;

template<size_t N>
const bool is_prime<N, N> = true;

template<size_t I>
const bool is_prime<1, I> = false;

template<size_t N>
class Residue {
private:
    long long number;

public:
    Residue() = default;

    Residue(int x) {
        if (x >= 0)
            number = x % N;
        else {
            number = (N - ((-x) % N)) % N;
        }
    }

    explicit operator int() { return number; }

    Residue &operator+=(const Residue &argument) {
        number += argument.number;
        number %= N;
        return *this;
    }

    Residue &operator-=(const Residue &argument) {
        number -= argument.number;
        if (number < 0) number = N + number;
        number %= N;

        return *this;
    }

    Residue &operator*=(const Residue &argument) {
        number *= argument.number;
        number %= N;
        return *this;
    }

    Residue operator-() const {
        Residue<N> newResidue(*this);
        newResidue.number = (N - number) % N;
        return newResidue;
    }

    Residue pow(size_t K) const;

    Residue &operator/=(const Residue &argument) {
        //std::cerr << N;
        static_assert(is_prime<N, 2>);
        *this *= argument.pow(N - 2);
        return *this;
    }

    bool equal(const Residue &argument) const {
        return number == argument.number;
    }

    long long num() {
        return number;
    }
};

template<size_t N>
std::ostream &operator<<(std::ostream &Ostream, Residue<N> input) {
    Ostream << input.num();
    return Ostream;
}

template<size_t N>
bool operator==(const Residue<N> &argument1, const Residue<N> &argument2) {
    return argument1.equal(argument2);
}

template<size_t N>
bool operator!=(const Residue<N> &argument1, const Residue<N> &argument2) {
    return !(argument1 == argument2);
}

template<size_t N>
Residue<N> operator+(const Residue<N> &argument1, const Residue<N> &argument2) {
    Residue<N> newResidue = argument1;
    newResidue += argument2;
    return newResidue;
}

template<size_t N>
Residue<N> operator-(const Residue<N> &argument1, const Residue<N> &argument2) {
    Residue<N> newResidue = argument1;
    newResidue -= argument2;
    return newResidue;
}

template<size_t N>
Residue<N> operator*(const Residue<N> &argument1, const Residue<N> &argument2) {
    Residue<N> newResidue = argument1;
    newResidue *= argument2;
    return newResidue;
}

template<size_t N>
Residue<N> operator/(const Residue<N> &argument1, const Residue<N> &argument2) {
    Residue<N> newResidue = argument1;
    newResidue /= argument2;
    return newResidue;
}

template<size_t N>
Residue<N> Residue<N>::pow(size_t K) const {
    if (K == 0) return Residue(1);
    if (K % 2 == 1)
        return pow(K - 1) * (*this);
    else {
        return pow(K / 2) * pow(K / 2);
    }
}

template<size_t N, size_t M, typename Field = Rational>

class Matrix {
private:
    std::vector<std::vector<Field>> entrails;

public:
    explicit Matrix(long long x) : entrails(std::vector<std::vector<Field>>(N, std::vector<Field>(M, Field(x)))) {}

    Matrix() : Matrix(0) {
        for (size_t i = 0; i < N; ++i) {
            entrails[i][i] = Field(1);
        }
    }

    Matrix(const std::vector<std::vector<Field>> &x) : entrails(x) {}

    Matrix(std::initializer_list<std::vector<Field>> matrix) {
        for (auto line: matrix) {
            entrails.push_back(line);
        }
    }

    std::vector<Field> &operator[](size_t i) { return entrails[i]; }

    std::vector<Field> operator[](size_t i) const { return entrails[i]; }

    Matrix<N, M, Field> &operator+=(const Matrix<N, M, Field> &argument) {
        for (size_t i = 0; i < N; i++) {
            for (size_t k = 0; k < M; k++) {
                entrails[i][k] += argument[i][k];
            }
        }
        return *this;
    }

    Matrix<N, M, Field> &operator-=(const Matrix<N, M, Field> &argument) {
        for (size_t i = 0; i < N; i++) {
            for (size_t k = 0; k < M; k++) {
                entrails[i][k] -= argument[i][k];
            }
        }
        return *this;
    }

    Matrix<N, M, Field> &operator*=(const Field &number) {
        for (size_t i = 0; i < N; i++) {
            for (size_t k = 0; k < M; k++) {
                entrails[i][k] *= number;
            }
        }
        return *this;
    }

    Matrix<N, M, Field> &operator*=(const Matrix<N, M, Field> &argument) {
        static_assert(N == M);
        Matrix<N, N> newMatrix = (*this) * argument;
        *this = newMatrix;
        return (*this);
    }

    std::vector<Field> getRow(size_t i) const { return entrails[i]; }

    std::vector<Field> getColumn(size_t i) const {
        std::vector<Field> answer(N);
        for (size_t k = 0; k < N; k++) {
            answer[k] = entrails[k][i];
        }
        return answer;
    }

    Matrix<M, N, Field> transposed() const {
        //std::cerr << "trans";
        Matrix<M, N, Field> newMatrix;
        for (size_t i = 0; i < M; i++) {
            for (size_t k = 0; k < N; k++) {
                newMatrix[i][k] = entrails[k][i];
            }
        }
        return newMatrix;
    }

    std::pair<Matrix<N, M, Field>, Field> gauss();

    void invertGauss();


    Field det() const {
        static_assert(N == M);
        //td::cerr << "det";
        Matrix newMatrix = *this;
        std::pair<Matrix<N, M, Field>, Field> res = newMatrix.gauss();

        return res.second;
    }

    size_t rank() const {
        //std::cerr << "rank";
        size_t answer = 0;
        Matrix newMatrix = *this;
        std::pair<Matrix<N, M, Field>, Field> res = newMatrix.gauss();
        Matrix gaussMatrix = res.first;
        for (size_t i = 0; i < N; i++) {
            for (size_t k = 0; k < M; k++) {
                if (gaussMatrix[i][k] != Field(0)) {
                    answer++;
                    break;
                }
            }
        }
        return answer;
    }

    Field trace() const {
        static_assert(N == M);

        //std::cerr << "trace";
        Field answer = Field(0);
        for (size_t i = 0; i < N; i++) {
            answer += entrails[i][i];
        }
        return answer;
    }

    void concatenate() {
        for (size_t i = 0; i < N; ++i) {
            entrails[i].resize(M * 2, 0);
            entrails[i][M + i] = Field(1);
        }
    }

    void invert() {
        static_assert(N == M);
        concatenate();

        gauss();
        //invertGauss();
        Matrix<N, M, Field> newMatrix;
        for (size_t i = 0; i < N; ++i) {
            for (size_t k = 0; k < N; ++k) {
                newMatrix[i][k] = entrails[i][k + N];
            }
        }
        *this = newMatrix;
    }

    Matrix<N, M, Field> inverted() const {
        //std::cerr << (*this);
        Matrix<N, M, Field> newMatrix = (*this);
        newMatrix.invert();
        return newMatrix;
    }

    friend int main();
};

template<size_t N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;

template<size_t N, size_t M, typename Field = Rational>
Matrix<N, M, Field> operator*(const Field &number, const Matrix<N, M, Field> &argument) {
    Matrix<N, M, Field> newMatrix = argument;
    newMatrix *= number;
    return newMatrix;
}

template<size_t N1, size_t M1, size_t N2, size_t M2, typename Field = Rational>
bool operator==(const Matrix<N1, M1, Field> &argument1,
                const Matrix<N2, M2, Field> &argument2) {
    if (N1 != N2 || M1 != M2) return false;
    for (size_t i = 0; i < N1; i++) {
        for (size_t k = 0; k < M1; k++) {
            if (argument1[i][k] != argument2[i][k]) return false;
        }
    }
    return true;
}

template<size_t N1, size_t M1, size_t N2, size_t M2, typename Field = Rational>
bool operator!=(const Matrix<N1, M1, Field> &argument1,
                const Matrix<N2, M2, Field> &argument2) {
    return !(argument1 == argument2);
}

template<size_t N, size_t M, typename Field = Rational>
Matrix<N, M, Field> operator+(const Matrix<N, M, Field> &argument1,
                              const Matrix<N, M, Field> &argument2) {
    Matrix<N, M, Field> newMatrix(argument1);
    newMatrix += argument2;
    return newMatrix;
}

template<size_t N, size_t M, typename Field = Rational>
Matrix<N, M, Field> operator-(const Matrix<N, M, Field> &argument1,
                              const Matrix<N, M, Field> &argument2) {
    Matrix<N, M, Field> newMatrix(argument1);
    newMatrix -= argument2;
    return newMatrix;
}

template<size_t N1, size_t M, size_t N2, typename Field = Rational>
Matrix<N1, N2, Field> operator*(const Matrix<N1, M, Field> &argument1,
                                const Matrix<M, N2, Field> &argument2) {
    Matrix<N1, N2, Field> newMatrix(0ll);
    for (size_t i = 0; i < N1; i++) {
        for (size_t k = 0; k < N2; k++) {
            for (size_t t = 0; t < M; t++) {
                newMatrix[i][k] += argument1[i][t] * argument2[t][k];
            }
        }
    }
    return newMatrix;
}

template<typename Field = Rational>

std::vector<Field> multiply(const std::vector<Field> &argument, const Field &number) {
    std::vector<Field> copyArgument = argument;
    for (size_t i = 0; i < argument.size(); i++) {
        copyArgument[i] *= number;
    }
    return copyArgument;
}

template<typename Field = Rational>
void deltaVector(std::vector<Field> &argument1,
                 const std::vector<Field> &argument2) {
    for (size_t i = 0; i < argument1.size(); i++) {
        argument1[i] -= argument2[i];
    }
}

template<typename Field = Rational>
void oneVector(std::vector<Field> &argument1, size_t first) {
    for (size_t i = first + 1; i < argument1.size(); i++) {
        argument1[i] /= argument1[first];
    }
    argument1[first] = Field(1);
}

template<size_t N, size_t M, typename Field = Rational>
std::ostream &operator<<(std::ostream &Ostream, const Matrix<N, M, Field> &input) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t k = 0; k < N; ++k) {
            Ostream << double(input[i][k]) << " ";
        }
        Ostream << std::endl;
    }
    return Ostream;
}

template<size_t N, size_t M, typename Field>
std::pair<Matrix<N, M, Field>, Field> Matrix<N, M, Field>::gauss() {
    Field det = Field(1);
    for (size_t i = 0; i < std::min(N, M); i++) {
        size_t nowIndex = i;
        while (nowIndex < N && entrails[nowIndex][i] == Field(0)) {
            nowIndex++;
        }
        if (nowIndex != N) {
            if (nowIndex != i) {
                det = -det;
                std::swap(entrails[i], entrails[nowIndex]);
            }
            det *= entrails[i][i];
            oneVector(entrails[i], i);
            for (size_t k = 0; k < N; k++) {
                if (k != i) {
                    deltaVector(entrails[k], multiply(entrails[i], entrails[k][i]));
                }
            }
        }

        //std::cout << (*this) << std::endl << std::endl;

    }
    size_t nowIndex = N;
    if (M > N) {
        while (entrails[N - 1][nowIndex] == Field(0))nowIndex++;
        if (nowIndex != M) {
            oneVector(entrails[N - 1], nowIndex);
            for (size_t k = 0; k < N - 1; k++) {
                deltaVector(entrails[k], multiply(entrails[N - 1], entrails[k][nowIndex]));
            }
        }
    }
    //std::cout << (*this) << std::endl << std::endl;
    return {*this, det};
}

template<size_t N, size_t M, typename Field>
void Matrix<N, M, Field>::invertGauss() {
    for (size_t i = N; i > 0; --i) {
        for (size_t k = i - 1; k > 0; --k) {
            entrails[k - 1][i - 1] -= entrails[i - 1][i - 1];
            //deltaVector(entrails[k-1], multiply(entrails[i-1], entrails[k-1][i-1]));
        }
    }
}
