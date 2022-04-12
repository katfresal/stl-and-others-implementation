#include <iostream>
#include <vector>


template<typename T>
class Deque {
private:
    static const size_t bucket_size = 8;
    size_t capacity = 0;
    T **meaning;
    size_t deque_size = 0;
    size_t first_bucket = 0;
    size_t last_bucket = 0;
    size_t index_in_first = 0;
    size_t index_in_last = 0;


public:

    template<bool IsConst>
    class common_iterator {
    private:
        T **mean;
        T *indicator;
        size_t index;

    public:
        explicit common_iterator(std::conditional_t<IsConst, T ** const, T **> mean, size_t index = 0)
                : mean(mean), indicator(mean[index / bucket_size]), index(index) {};

        std::conditional_t<IsConst, const T &, T &> operator*() {
            return indicator[index % bucket_size];
        }

        std::conditional_t<IsConst, const T *, T *> operator->() {
            return indicator + (index % bucket_size);
        }

        common_iterator<IsConst> &operator++() &{
            ++index;
            if (index % bucket_size == 0) {
                indicator = mean[index / bucket_size];
            }
            return *this;
        }


        common_iterator<IsConst> operator++(int) {
            common_iterator copy = *this;
            ++(*this);
            return copy;
        }

        common_iterator<IsConst> &operator--() &{
            if (index % bucket_size == 0) {
                index--;
                indicator = mean[index / bucket_size];
            } else --index;
            return *this;
        }

        common_iterator<IsConst> operator--(int) {
            common_iterator copy = *this;
            --(*this);
            return copy;
        }

        common_iterator<IsConst> &operator+=(int shift) &{
            if (shift < 0)
                *this -= -shift;
            else {
                index += shift;
                indicator = mean[index / bucket_size];
            }
            return *this;
        }

        common_iterator<IsConst> &operator-=(int shift) &{
            if (shift < 0)
                *this += -shift;
            else{
                index -= shift;
                indicator = mean[index / bucket_size];
            }
            return *this;
        }

        bool operator<(common_iterator<IsConst> argument) {
            return index < argument.index;
        }

        bool operator>=(common_iterator<IsConst> argument) {
            return !(*this < argument);
        }

        bool operator<=(common_iterator<IsConst> argument) {
            return argument >= *this;
        }

        bool operator>(common_iterator<IsConst> argument) {
            return argument < (*this);
        }

        bool operator==(common_iterator<IsConst> argument) {
            return index == argument.index;
        }

        bool operator!=(common_iterator<false> argument) {
            return index != argument.index;
        }

        common_iterator<IsConst> operator+(int shift) {
            auto copy = *this;
            copy += shift;
            return copy;
        }

        common_iterator<IsConst> operator-(int shift) {
            auto copy = *this;
            copy -= shift;
            return copy;
        }

        int operator-(const common_iterator<IsConst> &argument) {
            return index - argument.index;
        }

        friend Deque;
    };


    // template<bool IsConst>

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    //Deque() =  default;
    explicit Deque(size_t n) : capacity((3 * ((n - 1) / bucket_size + 1))), meaning(new T *[capacity]), deque_size(n),
                               first_bucket(((n - 1) / bucket_size + 1)),
                               last_bucket(2 * ((n - 1) / bucket_size + 1) - 1),
                               index_in_last((deque_size + bucket_size - 1) % bucket_size) {
        for (size_t i = 0; i < capacity; ++i) {
            meaning[i] = reinterpret_cast<T *>(new uint8_t[bucket_size * sizeof(T)]);
        }
    }

    Deque() :  capacity(3), meaning(new T *[capacity]), deque_size(0),
               first_bucket(1),
               last_bucket(1),
               index_in_last(0) {
        for (size_t i = 0; i < capacity; ++i) {
            meaning[i] = reinterpret_cast<T *>(new uint8_t[bucket_size * sizeof(T)]);
        }
    }


    Deque(int n, const T &x) : Deque(n) {
        //T* new_arr = reinterpret_cast<T*>(new char[first_bucket * sizeof(T)]);
        for (int i = 0; i < n; ++i) {
            try {
                new(meaning[first_bucket + i / bucket_size] + i % bucket_size) T(x);
            } catch (...) {
                for (int j = 0; j < i; ++j) {
                    (meaning[first_bucket + i / bucket_size] + i % bucket_size)->~T();
                }
                delete[] reinterpret_cast<uint8_t *>(meaning);
                throw;
            }

        }
    }

    Deque(const Deque<T> &argument) {
        capacity = argument.capacity;
        meaning = new T *[capacity];
        deque_size = argument.deque_size;
        first_bucket = argument.first_bucket;
        last_bucket = argument.last_bucket;
        index_in_first = argument.index_in_first;
        index_in_last = argument.index_in_last;
        for (size_t i = 0; i < capacity; ++i) {
            meaning[i] = reinterpret_cast<T *>(new uint8_t[bucket_size * sizeof(T)]);
        }

        for (size_t i = 0; i < deque_size; ++i) {
            try {
                new(meaning[first_bucket + (i + index_in_first) / bucket_size] +
                    ((i + index_in_first) % bucket_size)) T(argument[i]);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    (meaning[first_bucket + (i + index_in_first) / bucket_size] +
                     ((i + index_in_first) % bucket_size))->~T();
                }
                delete[] reinterpret_cast<uint8_t *>(meaning);
                throw;
            }

        }

    }

    void swap(Deque<T> argument) {
        if (this != &argument) {
            std::swap(capacity, argument.capacity);
            std::swap(meaning, argument.meaning);
            std::swap(deque_size, argument.deque_size);
            std::swap(first_bucket, argument.first_bucket);
            std::swap(last_bucket, argument.last_bucket);
            std::swap(index_in_first, argument.index_in_first);
            std::swap(index_in_last, argument.index_in_last);
        }
    }

    Deque &operator=(const Deque &argument) {
        swap(argument);
        return *this;
    }

    size_t size() const {
        return deque_size;
    }

    T &operator[](size_t index) {
        if (index < bucket_size - index_in_first)
            return meaning[first_bucket][index_in_first + index];
        return meaning[first_bucket + (index + index_in_first - bucket_size) / bucket_size + 1][
                (index - (bucket_size - index_in_first)) % bucket_size];
    }

    const T &operator[](size_t index) const {
        if (index < bucket_size - index_in_first)
            return meaning[first_bucket][index_in_first + index];
        return meaning[first_bucket + (index + index_in_first - bucket_size) / bucket_size + 1][
                (index - (bucket_size - index_in_first)) % bucket_size];
    }

    T &at(size_t index) {
        if (index >= 0 && index < deque_size) return (*this)[index];
        else throw std::out_of_range("out_of_range");
    }

    const T &at(size_t index) const {
        if (index >= 0 && index < deque_size) return (*this)[index];
        else throw std::out_of_range("out_of_range");
    }

    void reallocate(const T &x, size_t new_el) {
        size_t buben = (last_bucket - first_bucket + 1);
        T **copy = new T *[3 * buben];
        try {
            copy[new_el] = reinterpret_cast<T *>(new uint8_t[bucket_size * sizeof(T)]);
            if(new_el == 2 * buben) {
                new(copy[new_el]) T(x);
            } else {
                new(copy[new_el] + bucket_size - 1) T(x);
            }
            for (size_t i = 0; i < buben; ++i) {
                if (i != new_el) copy[i] = reinterpret_cast<T *>(new uint8_t[bucket_size * sizeof(T)]);
            }
            for (size_t i = buben; i < 2 * buben; ++i) {
                copy[i] = meaning[i + first_bucket - buben];
            }
            for (size_t i = buben * 2; i < 3 * buben; ++i) {
                if (i != new_el) copy[i] = reinterpret_cast<T *>(new uint8_t[bucket_size * sizeof(T)]);
            }
            delete[] meaning;
            meaning = copy;
            first_bucket = buben;
            last_bucket = 2 * buben - 1;
            if (new_el == 2 * buben) {
                ++last_bucket;
                index_in_last = 0;
            } else {
                --first_bucket;
                index_in_first = bucket_size - 1;
            }
            capacity = 3 * buben;
            //std::cout << "reallocated, capacity = " << capacity << "\n";
            //std::cout << std::endl;
        } catch (...) {
            delete[] reinterpret_cast <uint8_t *>(meaning[new_el]);
        }
    }

    void push_back(const T &x) {
        if (deque_size == 0){
            new(meaning[last_bucket] + (index_in_last)) T(x);
            deque_size++;
        }
        else {
            if (last_bucket * bucket_size + index_in_last < capacity * bucket_size - 1) {
                if (index_in_last < (bucket_size - 1)) {
                    new(meaning[last_bucket] + (index_in_last + 1)) T(x);
                    index_in_last++;
                } else {
                    new(meaning[last_bucket + 1] + 0) T(x);
                    last_bucket++;
                    index_in_last = 0;
                }
            } else {
                reallocate(x, 2 * (last_bucket - first_bucket + 1));
            }
            deque_size++;
        }
    }

    void push_front(const T &x) {
        if (first_bucket * bucket_size + index_in_first > 0) {
            if (index_in_first > 0) {
                new(meaning[first_bucket] + (index_in_first - 1)) T(x);
                index_in_first--;
            } else {
                new(meaning[first_bucket - 1] + (bucket_size - 1)) T(x);
                first_bucket--;
                index_in_first = bucket_size - 1;
            }
        } else {
            reallocate(x, last_bucket - first_bucket);
        }
        deque_size++;
    }

    void pop_back() {
        (*this)[deque_size - 1].~T();
        deque_size--;
        if (index_in_last > 0)index_in_last--;
        else {

            last_bucket--;
            index_in_last = bucket_size - 1;
        }
    }

    void pop_front() {
        (*this)[0].~T();
        deque_size--;
        if (index_in_first < bucket_size - 1)index_in_first++;
        else {

            first_bucket++;
            index_in_first = 0;
        }
    }

private:
    size_t ind(iterator it) {
        return it.index;
    }

public:
    iterator begin() {
        return iterator(meaning, first_bucket * bucket_size + index_in_first);
    }

    const_iterator cbegin() const {
        return const_iterator(meaning, first_bucket * bucket_size + index_in_first);
    }

    const_iterator begin() const {
        return cbegin();
    }

    iterator end() {
        return iterator(meaning, first_bucket * bucket_size + index_in_first + deque_size);
    }

    const_iterator cend() const {
        auto answer = const_iterator(meaning, last_bucket * bucket_size + index_in_last);
        ++answer;
        return answer;
    }

    const_iterator end() const {
        return cend();
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end() - 1);
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend() - 1);
    }

    const_reverse_iterator rbegin() const {
        return crbegin();
    }

    reverse_iterator rend() {
        return reverse_iterator(begin() - 1);
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin() - 1);
    }

    const_reverse_iterator rend() const {
        return crend();
    }

    void insert_reallocate() {
        T **copy = new T *[3 * capacity];

        for (size_t i = 0; i < capacity; ++i) {
            copy[i] = reinterpret_cast<T *>(new uint8_t[bucket_size * sizeof(T)]);
        }
        for (size_t i = capacity; i < 2 * capacity; ++i) {
            copy[i] = meaning[i - capacity];
        }
        for (size_t i = capacity * 2; i < 3 * capacity; ++i) {
            copy[i] = reinterpret_cast<T *>(new uint8_t[bucket_size * sizeof(T)]);
        }
        delete[] meaning;
        meaning = copy;
        first_bucket += capacity;
        last_bucket = 2 * capacity;
        index_in_last = 0;
        capacity = 3 * capacity;

    }

    void insert(iterator it, const T &x) {
        if (it == end())push_back(x);
        else {
            if (it == begin())push_front(x);
            else {
                if (last_bucket == capacity && index_in_last == bucket_size - 1) {
                    insert_reallocate();
                }

                for (iterator iter = end(); iter > it; --iter) {
                    *iter = *(iter - 1);
                }
                new(meaning[ ind(it)/ bucket_size ] + ind(it) % bucket_size) T(x);

                if (index_in_last == bucket_size - 1){
                    last_bucket++;
                    index_in_last = 0;
                }
                else{
                    index_in_last++;
                }
                deque_size++;
            }

        }
        std::cerr << "INSERT" << std::endl;
    }

    void erase(iterator it) {
        if (it == end())pop_back();
        else {
            if (it == begin())pop_front();
            else {

                for (iterator iter = it; iter < end(); ++iter) {
                    *(iter) = *(iter + 1);
                }
                //new(meaning[ ind(it)/ bucket_size ] + ind(it) % bucket_size) T(x);

                if (index_in_last == 0){
                    last_bucket--;
                    index_in_last = bucket_size - 1;
                }
                else{
                    index_in_last--;
                }
                deque_size--;
            }

        }
        std::cerr << "ERASE" << std::endl;
    }

    ~Deque() {
        for (size_t i = 0; i < capacity * bucket_size; ++i) {
            if (i >= first_bucket * bucket_size + index_in_first && i <= (last_bucket) * bucket_size + index_in_last) {
                (*this)[i].~T();
            }
        }
        for (size_t i = 0; i < capacity; ++i) {
            delete[] reinterpret_cast<uint8_t *> (meaning[i]);
        }
        delete[] meaning;
    }

};

template<typename T>
typename Deque<T>::iterator operator+(size_t n, typename Deque<T>::iterator &other) {
    return other + n;
}
