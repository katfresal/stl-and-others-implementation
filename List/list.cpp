#include <iostream>
#include <memory>
#include <list>


template<size_t N>
class StackStorage;

template<typename T, size_t N>

class alignas(::max_align_t) StackAllocator {
private:
    StackStorage<N> *now;
public:
    using value_type = T;
    template<typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };
    template<typename U>
    StackAllocator(const StackAllocator<U, N> &argument) noexcept: now(const_cast<StackStorage<N> *>(argument.now)) {}

    explicit StackAllocator(StackStorage<N> &argument) noexcept: now(&argument) {}

    StackAllocator() = delete;


    template<typename U, size_t M>
    StackAllocator &operator=(const StackAllocator<U, M> &argument) noexcept {
        now = argument.now;
        return *this;
    }

    template<typename U, size_t M>
    StackAllocator &operator==(const StackAllocator<U, M> &argument) {
        return now == argument.now;
    }

    template<typename U, size_t M>
    StackAllocator &operator!=(const StackAllocator<U, M> &argument) {
        return !(*this == argument);
    }


    T *allocate(size_t n) {
        size_t shift = (now->last - now->memory);
        now->last += (alignof(T) - shift % alignof(T)) % alignof(T);
//        size_t shift = (now->last - static_cast<uint8_t *>(nullptr)) % alignof(T);
//        if (shift) now->last += alignof(T) - shift;
        T *temp = reinterpret_cast<T *> (now->last);
  //      int ost = sizeof(T) % alignof(T);
        size_t size_with_alignment = sizeof(T);
//        if (ost) size_with_alignment += sizeof(T) + ((sizeof(T) - ost) % alignof(T));
        now->last += n * size_with_alignment;

//        ++Helper::allocate_calls;
//      allocate_call  std::cerr << "alloc\n";
        return temp;
    }

    void deallocate(T *ptr, size_t n) {
//        ++Helper::deallocate_calls;
//        std::cerr << "dealloc\n";
        if (reinterpret_cast<uint8_t *>(ptr + n) == now->last) {
            now->last = reinterpret_cast<uint8_t *>(ptr);
        }
    }

    template<typename U, size_t M>
    friend
    class StackAllocator;

};


template<size_t N>
class StackStorage {
private:
    uint8_t memory[N];
    uint8_t *last;
public:
//    uint8_t* get_cur() {
//        return last;
//    }
    StackStorage() : last(memory) {}

    StackStorage(const StackStorage &) = delete;
    //StackStorage operator& =()


    template<typename T, size_t M>
    friend
    class StackAllocator;
    friend int main();
};


template<typename T, typename Alloc = std::allocator<T>>
class List {
private:
    struct BaseNode {
        BaseNode *prev;
        BaseNode *next;

        BaseNode(BaseNode *fNode, BaseNode *sNode) : prev(fNode), next(sNode) {}

        BaseNode() : prev(nullptr), next(nullptr) {}

        virtual int getx() {
            return 0;
        };
        ~BaseNode() = default;
    };

    struct Node : BaseNode {
        T value;
        using BaseNode::prev;
        using BaseNode::next;

        Node(const BaseNode &arg, const T &val) : BaseNode(arg), value(val) {}

        Node(BaseNode *fNode, BaseNode *sNode, const T &val) : BaseNode(fNode, sNode), value(val) {}

        Node() : BaseNode(), value() {}

        explicit Node(const T &val) : BaseNode(nullptr, nullptr), value(val) {}
        ~Node() = default;
    };

    BaseNode *head;
    size_t size_list = 0;
    Alloc allocator;


    using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using basenode_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode>;
    using node_traits = std::allocator_traits<node_allocator>;
    using basenode_traits = std::allocator_traits<basenode_allocator>;

    basenode_allocator b_al;
    node_allocator n_al;

    void merge_basenode(size_t n) {
        try {
            for (size_t i = 0; i < n; i++) {
                push_back_null();
            }
        } catch (...) {
            clear2();
            throw;
        }
    }

    void merge_node(size_t n, const T &val) {
        try {
            for (size_t i = 0; i < n; i++) {
                push_back(val);
            }
        } catch (...) {
            clear2();
            throw;
        }
    }

    void create_head() {
        head = basenode_traits::allocate(b_al, 1);
        basenode_traits::construct(b_al, head, head, head);
    }

public:

    template<bool IsConst>
    class common_iterator {
    private:
        std::conditional_t<IsConst, const BaseNode *, BaseNode *> node_ptr;

        common_iterator<false> cast() {
            return {const_cast<BaseNode *>(node_ptr)};
        }


    public:

        using iterator_category = std::bidirectional_iterator_tag;
        using pointer = std::conditional_t<IsConst, const T *, T *>;
        using reference = std::conditional_t<IsConst, const T &, T &>;
        using const_reference = const T &;
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<IsConst, const T, T>;

        common_iterator(const BaseNode *argument) {
            node_ptr = const_cast<BaseNode *>(argument);
        }

        common_iterator &operator=(const common_iterator &argument) {
            node_ptr = argument.node_ptr;
            return *this;
        }

        operator common_iterator<true>() {
            return {node_ptr};
        }

        common_iterator(const common_iterator &argument) : node_ptr(argument.node_ptr) {}

        std::conditional_t<IsConst, const T &, T &> operator*() {
//            return (static_cast<std::conditional_t<IsConst, const Node *, Node *>>(node_ptr))->value;
            return (dynamic_cast<std::conditional_t<IsConst, const Node *, Node *>>(node_ptr))->value;
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


        bool operator!=(const common_iterator<IsConst>& argument) const {
            return !(*this == argument);
        }

        bool operator==(const common_iterator<IsConst>& argument) const {
            return node_ptr == argument.node_ptr;
        }


        friend List;
    };


    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


    iterator begin() {
        return iterator(head->next);
    }

    const_iterator cbegin() const {
        return const_iterator(head->next);
    }

    const_iterator begin() const {
        return cbegin();
    }

    iterator end() {
        return iterator(head);
    }

    const_iterator cend() const {
        return const_iterator(head);
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


    List() : size_list(0), allocator(Alloc()), b_al(Alloc()), n_al(Alloc()) {
        create_head();
    }

    explicit List(size_t n) : size_list(0), allocator(Alloc()), b_al(Alloc()), n_al(Alloc()) {
        create_head();

        merge_basenode(n);

    }

    List(size_t n, const T &val) : size_list(0), allocator(Alloc()), b_al(Alloc()), n_al(Alloc()) {
        create_head();
        merge_node(n, val);

    }

    explicit List(const Alloc &allocator_argument) : size_list(0), allocator(allocator_argument),
                                                     b_al(allocator_argument), n_al(allocator_argument) {
        create_head();
    }

    List(size_t n, const Alloc &allocator_argument) : size_list(0), allocator(allocator_argument),
                                                      b_al(allocator_argument), n_al(allocator_argument) {
        create_head();
        merge_basenode(n);
    }

    List(size_t n, const T &value, const Alloc &allocator_argument) : size_list(n),
                                                                      allocator(allocator_argument),
                                                                      b_al(allocator_argument),
                                                                      n_al(allocator_argument) {
        create_head();

        merge_node(n, value);

    }

    void pop_back() {
        erase(--end());
    }

    void clear() {
        while (size_list != 0) {
            pop_back();
        }
        //basenode_traits::destroy(b_al, (head));
        //basenode_traits::deallocate(b_al, (head), 1);
    }

    void clear2(){
        while (size_list != 0) {
            pop_back();
        }
        //basenode_traits::destroy(b_al, (head));
//        basenode_traits::deallocate(b_al, (head), 1);
    }

    List(const List<T, Alloc> &argument) : size_list(0), allocator(
            std::allocator_traits<Alloc>::select_on_container_copy_construction(argument.allocator)),
                                           b_al(basenode_traits::select_on_container_copy_construction(argument.b_al)),
                                           n_al(node_traits::select_on_container_copy_construction(argument.n_al)) {
        create_head();
        for (auto it = argument.begin(); it != argument.end(); ++it) {
            try {
                push_back(*it);
            } catch (...) {
                clear2();
                throw;
            }
        }

    }

    List &operator=(const List &argument) {
        if (this == &argument) {
            return *this;
        }

        //size_list = 0;
        List copy(std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value ? argument.allocator : allocator);
        for (auto it = argument.begin(); it != argument.end(); ++it) {
            copy.push_back(*it);

        }
        std::swap(head, copy.head);
        std::swap(allocator, copy.allocator);
        std::swap(n_al, copy.n_al);
        std::swap(b_al, copy.b_al);
        std::swap(size_list, copy.size_list);
        return *this;
    }


    size_t size() const {
        return size_list;
    }

    Alloc get_allocator() const {
        return allocator;
    }


    void pop_front() {

        //auto it = begin();
        erase(begin());
    }

    void push_back(const T &val) {

        //auto it = cend();

        insert(end(), val);


    }

    void push_back_null() {
        //auto it = cend();

        insert_null(end());

    }

    void push_front(const T &val) {

        //auto it = begin();

        insert(begin(), val);


    }


    template<bool IsConst>
    void insert(common_iterator<IsConst> iter, const T &val) {
        iterator it = iter.cast();
        Node *new_node = node_traits::allocate(n_al, 1);
        try {
            node_traits::construct(n_al, new_node, val);
            BaseNode *temp = it.node_ptr->prev;
            temp->next = new_node;
            new_node->prev = temp;
            new_node->next = it.node_ptr;
            it.node_ptr->prev = new_node;
            size_list++;
        }
        catch (...) {
//            node_traits::destroy(n_al, (new_node));
            node_traits::deallocate(n_al, new_node, 1);
            throw;
        }


    }

    template<bool IsConst>
    void insert_null(common_iterator<IsConst> iter) {
        iterator it = iter.cast();
        Node *new_node = node_traits::allocate(n_al, 1);
        try {

            node_traits::construct(n_al, new_node);
            BaseNode *temp = it.node_ptr->prev;
            temp->next = new_node;
            new_node->prev = temp;
            new_node->next = it.node_ptr;
            it.node_ptr->prev = new_node;
            size_list++;
        }
        catch (...) {
//            node_traits::destroy(n_al, (new_node));
            node_traits::deallocate(n_al, (new_node), 1);
            throw;
        }


    }


    template<bool IsConst>
    void erase(common_iterator<IsConst> iter) {
        iterator it = iter.cast();
        Node* this_node = dynamic_cast<Node *>(it.node_ptr);
        if (it.node_ptr == head) return;
        BaseNode *temp = it.node_ptr->next;
        it.node_ptr->prev->next = temp;
        temp->prev = it.node_ptr->prev;
        node_traits::destroy(n_al,  this_node);
        node_traits::deallocate(n_al, this_node, 1);
        size_list--;
    }


    ~List() {
        while (size_list > 0) {
            pop_back();
        }
        basenode_traits::destroy(b_al, head);
        basenode_traits::deallocate(b_al, head, 1);
    }
};
