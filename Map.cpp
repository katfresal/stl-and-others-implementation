#include <iostream>
#include <vector>

template<typename T, typename Alloc = std::allocator<T>>
class List {

private:

    template<typename Key, typename Value, typename Hash, typename Equal,
            typename Allocator>
    friend
    class UnorderedMap;

    struct alignas(max_align_t) BaseNode {
        BaseNode *prev;
        BaseNode *next;

        BaseNode(BaseNode *fNode, BaseNode *sNode) : prev(fNode), next(sNode) {}

        BaseNode() : prev(nullptr), next(nullptr) {}

        virtual int getx() { return 0; };

        BaseNode(const BaseNode &node) : prev(node.prev), next(node.next) {}

        BaseNode(BaseNode &&node) noexcept: prev(node.prev), next(node.next) {
            node.prev = nullptr;
            node.next = nullptr;
        }

        ~BaseNode() = default;

        BaseNode &operator=(const BaseNode &node) = default;
    };

    struct Node : BaseNode {
        T value;
        size_t module_hash;
        using BaseNode::next;
        using BaseNode::prev;

        Node(const BaseNode &arg, const T &val, size_t hash)
                : BaseNode(arg), value(val), module_hash(hash) {}

        Node(BaseNode *fNode, BaseNode *sNode, const T &val, size_t hash)
                : BaseNode(fNode, sNode), value(val), module_hash(hash) {}

        Node(const T &val, size_t hash) : value(val), module_hash(hash) {}

        template<class... Args>
        Node(Args &&... args) : value(std::forward<Args>(args)...) {}

        Node(const Node &) = default;

        ~Node() = default;
    };



    using node_allocator =
            typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using basenode_allocator =
            typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode>;
    using node_traits = std::allocator_traits<node_allocator>;
    using basenode_traits = std::allocator_traits<basenode_allocator>;

    BaseNode head = BaseNode(&head, &head);
    size_t size_list = 0;
    Alloc allocator;
    basenode_allocator basenode_alloc;
    node_allocator node_alloc;


    void merge_basenode(size_t n);

    void merge_node(size_t n, const T &val);

    void create_head();

    void push_back_null() {
        insert_null(end());
    }

public:
    template<bool IsConst>
    class common_iterator {
        friend List;

        template<typename Key, typename Value, typename Hash, typename Equal,
                typename Allocator>
        friend
        class UnorderedMap;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using pointer = std::conditional_t<IsConst, const T *, T *>;
        using reference = std::conditional_t<IsConst, const T &, T &>;
        using const_reference = const T &;
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<IsConst, const T, T>;

        common_iterator(const BaseNode *other) : node_ptr(const_cast<BaseNode *>(other)) {
        }

        common_iterator(const common_iterator &other)
                : node_ptr(other.node_ptr) {}

        operator common_iterator<true>() {
            return {node_ptr};
        }

        common_iterator &operator=(const common_iterator &other) {
            node_ptr = other.node_ptr;
            return *this;
        }

        std::conditional_t<IsConst, const T &, T &> operator*() {
            return (dynamic_cast<std::conditional_t<IsConst, const Node *, Node *>>(
                    node_ptr))
                    ->value;
        }

        std::conditional_t<IsConst, const T *, T *> operator->() {
            return &(*(*this));
        }


        common_iterator<IsConst> &operator++() {
            node_ptr = node_ptr->next;
            return *this;
        }

        common_iterator operator++(int) {
            common_iterator copy = *this;
            ++(*this);
            return copy;
        }

        common_iterator &operator--() {
            node_ptr = node_ptr->prev;
            return *this;
        }

        common_iterator operator--(int) {
            common_iterator copy = *this;
            --(*this);
            return copy;
        }

        bool operator==(const common_iterator<IsConst> &other) const {
            return (node_ptr == other.node_ptr);
        }

        bool operator!=(const common_iterator<IsConst> &other) const {
            return !(*this == other);
        }


    private:
        std::conditional_t<IsConst, const BaseNode *, BaseNode *> node_ptr;

        common_iterator<false> cast() {
            return {const_cast<BaseNode *>(node_ptr)};
        }

        size_t get_hash() const {
            return static_cast<const Node *>(node_ptr)->module_hash;
        }

    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List() : size_list(0), allocator(Alloc()), basenode_alloc(Alloc()), node_alloc(Alloc()) {}

    explicit List(size_t n)
            : size_list(0), allocator(Alloc()), basenode_alloc(Alloc()), node_alloc(Alloc()) {
        merge_basenode(n);
    }

    List(size_t n, const T &val)
            : size_list(0), allocator(Alloc()), basenode_alloc(Alloc()), node_alloc(Alloc()) {
        merge_node(n, val);
    }

    explicit List(const Alloc &allocator_other)
            : size_list(0),
              allocator(allocator_other),
              basenode_alloc(allocator_other),
              node_alloc(allocator_other) {}

    List(size_t n, const Alloc &allocator_other)
            : size_list(0),
              allocator(allocator_other),
              basenode_alloc(allocator_other),
              node_alloc(allocator_other) {
        merge_basenode(n);
    }

    List(size_t n, const T &value, const Alloc &allocator_other)
            : size_list(n),
              allocator(allocator_other),
              basenode_alloc(allocator_other),
              node_alloc(allocator_other) {
        merge_node(n, value);
    }

    List(const List<T, Alloc> &other);

    List(List<T, Alloc> &&other) noexcept;

    ~List() {
        clear();
    }

    List &operator=(List &&other) noexcept;

    List &operator=(const List &other);


    iterator begin() {
        return iterator(head.next);
    }

    const_iterator cbegin() const {
        return const_iterator(head.next);
    }

    const_iterator begin() const {
        return cbegin();
    }

    iterator end() {
        return iterator(&head);
    }

    const_iterator cend() const {
        return const_iterator(&head);
    }

    const_iterator end() const {
        return cend();
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator rbegin() const {
        return crbegin();
    }

    reverse_iterator rend() {
        auto ans = begin();
        return reverse_iterator(--ans);
    }

    const_reverse_iterator crend() const {
        auto ans = cbegin();
        return const_reverse_iterator(--ans);
    }

    const_reverse_iterator rend() const {
        return crend();
    }


    bool empty() {
        return size_list == 0;
    }

    void pop_back() {
        erase(--end());
    }

    void clear() {
        while (!empty()) {
            pop_back();
        }
    }

    size_t size() const {
        return size_list;
    }

    Alloc get_allocator() const {
        return allocator;
    }

    void pop_front() {
        erase(begin());
    }

    void push_back(const T &val, size_t arg_hash) {
        insert(end(), val, arg_hash);
    }


    void push_front(const T &val, size_t arg_hash) {
        insert(begin(), val, arg_hash);
    }

    template<bool IsConst>
    iterator insert(common_iterator<IsConst> iter, const T &val, size_t hash);

    iterator erase(const_iterator iter);

    template<class... Args>
    iterator emplace(iterator iter, Args &&... args);

    void swap(List &other);

private:

    template<bool IsConst>
    void insert_null(common_iterator<IsConst> iter, size_t hash);

};

template<typename Key, typename Value, typename Hash = std::hash<Key>,
        typename Equal = std::equal_to<Key>,
        typename Alloc = std::allocator<std::pair<const Key, Value>>>

class UnorderedMap {

private:
    using NodeType = std::pair<const Key, Value>;
    using list_node = typename List<NodeType, Alloc>::Node;
    using node_allocator =
            typename std::allocator_traits<Alloc>::template rebind_alloc<list_node>;
    using list_iterator = typename List<NodeType, Alloc>::iterator;
    using const_list_iterator = typename List<NodeType, Alloc>::const_iterator;
    using iterator_allocator = typename std::allocator_traits<
            Alloc>::template rebind_alloc<list_iterator>;

    List<NodeType, Alloc> chain;
    std::vector<list_iterator, iterator_allocator> vector_hash;
    size_t number_node;
    Equal comp;
    Hash hash;
    node_allocator allocator;
    float max_factor = 0.8;
    friend List<NodeType>;

public:
    UnorderedMap()
            : chain(),
              vector_hash(8, chain.end()),
              number_node(0),
              comp(),
              hash(),
              allocator() {}

    UnorderedMap(const Alloc &allocator)
            : chain(),
              vector_hash(8, chain.end()),
              number_node(0),
              comp(),
              hash(),
              allocator(allocator) {};

    UnorderedMap(UnorderedMap &&other)
            : chain(std::move(other.chain)),
              vector_hash(std::move(other.vector_hash)),
              number_node(other.number_node),
              comp(std::move(other.comp)),
              hash(std::move(other.hash)),
              allocator(std::move(other.allocator)) {
        other.number_node = 0;
    }

    UnorderedMap(const UnorderedMap &other)
            : chain(
            std::allocator_traits<Alloc>::select_on_container_copy_construction(
                    other.allocator)),
              vector_hash(other.vector_hash.size(), chain.end(),
                          chain.get_allocator()),
              number_node(other.number_node),
              comp(other.comp),
              hash(other.hash),
              allocator(chain.get_allocator()) {
        for (const auto &element: other) {
            insert(element);
        }
    }

    ~UnorderedMap() = default;

    void swap(UnorderedMap &&other);

    UnorderedMap &operator=(UnorderedMap &&other) noexcept;

    UnorderedMap &operator=(const UnorderedMap &other);

    list_iterator begin() {
        return chain.begin();
    }

    const_list_iterator cbegin() const {
        return chain.cbegin();
    }

    const_list_iterator begin() const {
        return cbegin();
    }

    list_iterator end() {
        return chain.end();
    }

    const_list_iterator cend() const {
        return chain.cend();
    }

    const_list_iterator end() const {
        return cend();
    }

    size_t size() const {
        return chain.size();
    }

    template<typename Arg>
    Value &at(Arg &&other_key);

    const Value &at(const Key &other_key) const;

    Value &operator[](const Key &other_key);

    template<typename Arg>
    list_iterator find(Arg &&arg_key);

    std::pair<list_iterator, bool> insert(const Key &other_key,
                                          const Value &other_val);

    std::pair<list_iterator, bool> insert(const NodeType &arg) {
        return insert(arg.first, arg.second);
    }

    template<class Args>
    std::pair<list_iterator, bool> insert(Args &&arg) {
        return emplace(std::forward<Args>(arg));
    }

    std::pair<list_iterator, bool> insert(Key &&other_key,
                                          Value &&other_val);

    template<class input_iterator>
    void insert(input_iterator start, input_iterator finish);

    template<class... Args>
    std::pair<list_iterator, bool> emplace(Args &&... args);

    void erase(const_list_iterator it);

    void erase(const_list_iterator start, const_list_iterator finish) {
        for (auto i = start; i != finish; ++i) {
            erase(i);
        }
    }

    float load_factor() const {
        return static_cast<float>(chain.size() / vector_hash.size());
    }

    float max_load_factor() const noexcept {
        return max_factor;
    }

    void max_load_factor(float load_factor) {
        max_factor = load_factor;
    }

    void new_load_factor() {
        float new_load = static_cast<float>(size() + 1) / vector_hash.size();
        if (new_load > max_factor) {
            reserve(static_cast<size_t>(2 * vector_hash.size() + 1));
        }
    }

    void reserve(size_t count);

    void rehash();

};

template<typename T, typename Alloc>
template<class... Args>
typename List<T, Alloc>::iterator List<T, Alloc>::emplace(List::iterator iter,
                                                          Args &&... args) {
    iterator it = iter.cast();
    Node *new_node = node_traits::allocate(node_alloc, 1);
    try {
        node_traits::construct(node_alloc, new_node, std::forward<Args>(args)...);
        BaseNode *temp = it.node_ptr->prev;
        temp->next = new_node;
        new_node->prev = temp;
        new_node->next = it.node_ptr;
        it.node_ptr->prev = new_node;
        size_list++;
        return {new_node};
    } catch (...) {
        node_traits::deallocate(node_alloc, new_node, 1);
        throw;
    }
}

template<typename T, typename Alloc>
List<T, Alloc>::List(const List<T, Alloc> &other)
        : size_list(0),
          allocator(
                  std::allocator_traits<Alloc>::select_on_container_copy_construction(
                          other.allocator)),
          basenode_alloc(basenode_traits::select_on_container_copy_construction(
                  other.basenode_alloc)),
          node_alloc(node_traits::select_on_container_copy_construction(other.node_alloc)) {
    for (auto it = other.begin(); it != other.end(); ++it) {
        try {
            push_back(*it, it.get_hash());
        } catch (...) {
            clear();
            throw;
        }
    }
}

template<typename T, typename Alloc>
List<T, Alloc>::List(List<T, Alloc> &&other) noexcept
        : head(other.head),
          size_list(other.size_list),
          allocator(std::move(other.allocator)),
          basenode_alloc(std::move(other.basenode_alloc)),
          node_alloc(std::move(other.node_alloc)) {
    other.size_list = 0;
    other.head.next->prev = &head;
    other.head.prev->next = &head;
    other.head = {&other.head, &other.head};
}

template<typename T, typename Alloc>
List<T, Alloc> &List<T, Alloc>::operator=(List &&other) noexcept {
    size_list = other.size_list;
    head = other.head;
    other.head.next->prev = &head;
    other.head.prev->next = &head;
    other.head = {&other.head, &other.head};

    other.size_list = 0;
    if (std::allocator_traits<
            Alloc>::propagate_on_container_move_assignment::value) {
        allocator = std::move(other.allocator);
        node_alloc = std::move(other.node_alloc);
        basenode_alloc = std::move(other.basenode_alloc);
    }
    return *this;
}

template<typename T, typename Alloc>
void List<T, Alloc>::swap(List &other) {
    if (size() == 0 && other.size() != 0) {
        other.head.prev->next = &head;
        other.head.next->prev = &head;
        head.next = other.head.next;
        head.prev = other.head.prev;
        other.head.prev = &other.head;
        other.head.next = &other.head;
    } else {
        if (size() != 0 && other.size() == 0) {
            head.prev->next = &other.head;
            head.next->prev = &other.head;
            other.head.next = head.next;
            other.head.prev = head.prev;
            head.prev = &head;
            head.next = &head;
        } else {
            if (size() != 0 && other.size() != 0) {
                std::swap(head.prev->next, other.head.prev->next);
                std::swap(head.next->prev, other.head.next->prev);
                std::swap(head.next, other.head.next);
                std::swap(head.prev, other.head.prev);
            }
        }
    }

    std::swap(allocator, other.allocator);
    std::swap(node_alloc, other.node_alloc);
    std::swap(basenode_alloc, other.basenode_alloc);
    std::swap(size_list, other.size_list);
}

template<typename T, typename Alloc>
List<T, Alloc> &List<T, Alloc>::operator=(const List &other) {
    if (this == &other) {
        return *this;
    }
    List
    copy(std::allocator_traits<
            Alloc>::propagate_on_container_copy_assignment::value
         ? other.allocator
         : allocator);
    for (auto it = other.begin(); it != other.end(); ++it) {
        copy.push_back(*it, it.get_hash());
    }

    swap(copy);
    return *this;
}

template<typename T, typename Alloc>
template<bool IsConst>
void List<T, Alloc>::insert_null(List::common_iterator<IsConst> iter,
                                 size_t hash) {
    iterator it = iter.cast();
    Node *new_node = node_traits::allocate(node_alloc, 1);
    try {
        node_traits::construct(node_alloc, new_node, hash);
        BaseNode *temp = it.node_ptr->prev;
        temp->next = new_node;
        new_node->prev = temp;
        new_node->next = it.node_ptr;
        it.node_ptr->prev = new_node;
        size_list++;
    } catch (...) {
        node_traits::deallocate(node_alloc, (new_node), 1);
        throw;
    }
}

template<typename T, typename Alloc>
template<bool IsConst>
typename List<T, Alloc>::iterator List<T, Alloc>::insert(
        List::common_iterator<IsConst> iter, const T &val, size_t hash) {
    iterator it = iter.cast();
    Node *new_node = node_traits::allocate(node_alloc, 1);
    try {
        node_traits::construct(node_alloc, new_node, val, hash);
        BaseNode *temp = it.node_ptr->prev;
        temp->next = new_node;
        new_node->prev = temp;
        new_node->next = it.node_ptr;
        it.node_ptr->prev = new_node;
        size_list++;
        return {new_node};
    } catch (...) {
        node_traits::deallocate(node_alloc, new_node, 1);
        throw;
    }
}

template<typename T, typename Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::erase(
        List::const_iterator iter) {
    iterator it = iter.cast();
    Node *this_node = dynamic_cast<Node *>(it.node_ptr);
    if (it.node_ptr == &head) return {end()};
    BaseNode *temp = it.node_ptr->next;
    it.node_ptr->prev->next = temp;
    temp->prev = it.node_ptr->prev;
    node_traits::destroy(node_alloc, this_node);
    node_traits::deallocate(node_alloc, this_node, 1);
    size_list--;
    return {temp};
}

template<typename T, typename Alloc>
void List<T, Alloc>::merge_basenode(size_t n) {
    try {
        for (size_t i = 0; i < n; i++) {
            push_back_null();
        }
    } catch (...) {
        clear();
        throw;
    }
}

template<typename T, typename Alloc>
void List<T, Alloc>::merge_node(size_t n, const T &val) {
    try {
        for (size_t i = 0; i < n; i++) {
            push_back(val);
        }
    } catch (...) {
        clear();
        throw;
    }
}

template<typename T, typename Alloc>
void List<T, Alloc>::create_head() {
    head = basenode_traits::allocate(basenode_alloc, 1);
    basenode_traits::construct(basenode_alloc, head, head, head);
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::swap(
        UnorderedMap &&other) {
    vector_hash = std::move(other.vector_hash);
    chain.clear();
    chain = std::move(other.chain);
    hash = std::move(other.hash);
    comp = std::move(other.comp);
    number_node = std::move(other.number_node);
    max_factor = std::move(other.max_factor);
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>
&UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(
        UnorderedMap &&other) noexcept {
    allocator = std::allocator_traits<
            Alloc>::propagate_on_container_move_assignment::value
                ? std::move(other.allocator)
                : allocator;
    swap(std::move(other));
    return *this;
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>
&UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(
        const UnorderedMap &other) {
    UnorderedMap cop = other;
    allocator = std::allocator_traits<
            Alloc>::propagate_on_container_copy_assignment::value
                ? other.allocator
                : allocator;
    swap(std::move(cop));
    return *this;
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
template<typename Arg>
Value &UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(Arg &&other_key) {
    size_t true_hash = hash(std::forward<Arg>(other_key)) % vector_hash.size();
    list_iterator temp = vector_hash[true_hash];
    if (temp != chain.end()) {
        while ((temp.get_hash()) % vector_hash.size() == true_hash &&
               temp != chain.end()) {
            if (comp(temp->first, std::forward<Arg>(other_key))) {
                return temp->second;
            }
            temp++;
        }
    }
    throw std::out_of_range("out_of_range");
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
const Value &UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(
        const Key &other_key) const {
    size_t true_hash = hash(other_key) % vector_hash.size();
    list_iterator temp = vector_hash[true_hash];
    if (chain.end() != temp) {
        while ((temp.get_hash()) % vector_hash.size() == true_hash &&
               chain.end() != temp) {
            if (comp(temp->first, other_key)) {
                return temp->second;
            }
            temp++;
        }
    }
    throw std::out_of_range("out_of_range");
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
Value &UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[](
        const Key &other_key) {
    try {
        return at(other_key);
    } catch (std::out_of_range &) {
        new_load_factor();
        return (insert(other_key, Value()).first)->second;
    }
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
template<typename Arg>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::list_iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(Arg &&arg_key) {
    size_t true_hash = hash(std::forward<Arg>(arg_key)) % vector_hash.size();
    list_iterator temp = vector_hash[true_hash];
    if (temp != chain.end()) {
        while ((temp.get_hash()) % vector_hash.size() == true_hash &&
               temp != chain.end()) {
            if (comp(temp->first, std::forward<Arg>(arg_key))) {
                return temp;
            }
            temp++;
        }
    }
    return chain.end();
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::list_iterator,
        bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(
        const Key &other_key, const Value &other_val) {
    size_t true_hash = hash(other_key) % vector_hash.size();
    list_iterator temp = vector_hash[true_hash];
    if (temp != chain.end()) {
        while ((temp.get_hash()) % vector_hash.size() == true_hash &&
               temp != chain.end()) {
            if (comp(temp->first, other_key)) return {temp, false};
            ++temp;
        }
        new_load_factor();
        return {
                chain.insert(temp, {other_key, other_val}, hash(other_key)),
                true};
    }
    new_load_factor();
    chain.push_front({other_key, other_val}, hash(other_key));
    vector_hash[true_hash] = chain.begin();
    number_node++;
    return {begin(), true};
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::list_iterator,
        bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(Key &&other_key,
                                                     Value &&other_val) {
    new_load_factor();
    size_t true_hash = hash(other_key) % vector_hash.size();
    list_iterator temp = vector_hash[true_hash];
    if (temp != chain.end()) {
        while ((temp.get_hash()) % vector_hash.size() == true_hash &&
               temp != chain.end()) {
            if (comp(temp->first, other_key)) return {temp, false};
            ++temp;
        }
        new_load_factor();

        return {chain.insert(temp, std::move(other_key), std::move(other_val),
                             hash(other_key)),
                true};
    }
    new_load_factor();
    chain.push_front(
            {std::forward<Key>(other_key), std::forward<Value>(other_val)},
            hash(other_key));
    vector_hash[true_hash] = chain.begin();
    number_node++;
    return {begin(), true};
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
template<class input_iterator>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(
        input_iterator start, input_iterator finish) {
    for (auto i = start; i != finish; ++i) {
        insert(*i);
    }
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
template<class... Args>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::list_iterator,
        bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::emplace(Args &&... args) {
    list_node *new_node =
            std::allocator_traits<node_allocator>::allocate(allocator, 1);
    char *temp_ptr = reinterpret_cast<char *>(new_node) + 3 * sizeof(list_node *);

    try {
        std::allocator_traits<node_allocator>::construct(
                allocator, reinterpret_cast<NodeType *>(temp_ptr),
                std::forward<Args>(args)...);
    } catch (...) {
        std::allocator_traits<node_allocator>::deallocate(allocator, new_node, 1);
        throw;
    }
    try {
        new_node->module_hash = hash(new_node->value.first);
    } catch (...) {
        std::allocator_traits<node_allocator>::destroy(allocator, new_node);
        std::allocator_traits<node_allocator>::deallocate(allocator, new_node, 1);
        throw;
    }

    size_t true_hash = new_node->module_hash % vector_hash.size();
    list_iterator temp = vector_hash[true_hash];
    if (temp != chain.end()) {
        while ((temp.get_hash()) % vector_hash.size() == true_hash &&
               temp != chain.end()) {
            if (comp(temp->first, new_node->value.first)) return {temp, false};
            ++temp;
        }
        new_load_factor();
        return {
                chain.emplace(temp, std::move(const_cast<Key &>(new_node->value.first)), std::move(new_node->value.second)),
                true};
    }
    new_load_factor();
    auto it = chain.emplace(chain.begin(),
                            std::move(const_cast<Key &>(new_node->value.first)),
                            std::move(new_node->value.second));
    dynamic_cast<list_node *>(it.node_ptr)->module_hash =
            hash(new_node->value.first);
    vector_hash[true_hash] = chain.begin();
    number_node++;
    return {begin(), true};
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(
        UnorderedMap::const_list_iterator it) {
    size_t true_hash = hash(it->first) % vector_hash.size();
    list_iterator temp = vector_hash[true_hash];
    if (it == temp) {
        list_iterator new_it = chain.erase(it);
        if (new_it.get_hash() % vector_hash.size() == true_hash) {
            vector_hash[true_hash] = new_it;
        } else {
            vector_hash[true_hash] = chain.end();
        }
    } else
        chain.erase(it);
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::reserve(size_t count) {
    if (count > vector_hash.size()) {
        size_t new_count =
                static_cast<size_t>(static_cast<float>(count) / max_factor + 1);
        vector_hash.clear();
        vector_hash.resize(new_count, chain.end());
        rehash();
    }
}

template<typename Key, typename Value, typename Hash, typename Equal,
        typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::rehash() {
    List<NodeType, Alloc> new_list;
    chain.swap(new_list);

    for (list_iterator it = new_list.begin(); it != new_list.end(); ++it) {
        emplace(std::move(const_cast<Key &>(it->first)), std::move(it->second));
    }
    new_list.head = {&new_list.head, &new_list.head};
    new_list.size_list = 0;
}

