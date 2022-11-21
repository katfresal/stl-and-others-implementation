#include <iostream>
#include <memory>
#include <optional>
#include <type_traits>

namespace blocks {

    struct Counter {
        size_t shared_count = 1;
        size_t weak_count = 1;

        void swap(Counter& argument) {
            std::swap(shared_count, argument.shared_count);
            std::swap(weak_count, argument.weak_count);
        }
    };

    struct BaseBlock {
        Counter count;

        virtual void delete_object() = 0;

        virtual void deallocate() = 0;

        virtual ~BaseBlock() = default;
    };

    template<typename T, typename Deleter = std::default_delete<T>, typename Allocator = std::allocator<T>>
    struct ControlPointerBlock : BaseBlock {
        using BaseBlock::count;

        T* object;
        Deleter del;
        Allocator alloc;

        using ControlPointerAllocator =
                typename std::allocator_traits<Allocator>::template rebind_alloc<ControlPointerBlock<T, Deleter, Allocator>>;
        using PointerTraits = std::allocator_traits<ControlPointerAllocator>;

        ControlPointerBlock(T *pointer,const Deleter& delet, const Allocator& allocator) : object(pointer), del(delet),
                                                                                           alloc(allocator) {}

        ControlPointerBlock(T *pointer, const Deleter& delet, const Allocator& allocator, size_t shared_size, size_t weak_size)
                : ControlPointerBlock(pointer, delet, allocator) {
            count.shared_count = shared_size;
            count.weak_count = weak_size;
        }

        void delete_object() {
            if (object) {
                del(object);
                object = nullptr;
            }
        }

        void deallocate() {
            ControlPointerAllocator pointer_alloc(alloc);
            if (std::is_same_v<Deleter, std::default_delete<T>>){
                PointerTraits::destroy(pointer_alloc, this);
            }
            PointerTraits::deallocate(pointer_alloc, this, 1);
        }

    };

    template<typename T, typename Allocator = std::allocator<T>>
    struct ControlArgsBlock : BaseBlock {
        using BaseBlock::count;

        std::optional<T> val;
        Allocator alloc;

        using ControlArgsAllocator =
                typename std::allocator_traits<Allocator>::template rebind_alloc<ControlArgsBlock<T, Allocator>>;
        using ArgsTraits = std::allocator_traits<ControlArgsAllocator>;

        void delete_object() {
            if (val) val.reset();
        }

        template<class... Args>
        ControlArgsBlock(const Allocator& allocator, Args&& ... args): alloc(allocator){
            val.template emplace(std::forward<Args>(args)...);
        }

        void deallocate() {
            ControlArgsAllocator arg_alloc(alloc);
            ArgsTraits::destroy(arg_alloc, this);
            ArgsTraits::deallocate(arg_alloc, this, 1);
        }
    };
}

template<typename T>
class WeakPtr;

template<typename T>
class SharedPtr {
    template<typename ... Args>
    friend SharedPtr<T> make_shared(Args&& ... args);

    template<typename  Y, typename ... Args, typename Allocator>
    friend SharedPtr<Y> allocateShared(const Allocator& alloc, Args&& ... args);

    friend class WeakPtr<T>;

    template<typename Y>
    friend
    class SharedPtr;

    template<typename Y>
    friend
    class WeakPtr;

private:

    blocks::BaseBlock *block = nullptr;
    T* ptr = nullptr;

    SharedPtr(T& arg_ptr, blocks::BaseBlock *argument) : block(argument), ptr(&arg_ptr) {}

public:

    SharedPtr() = default;

    SharedPtr(const WeakPtr<T>& argument) : block(argument.base_block), ptr(nullptr) {
        ++block->count.shared_count;
    }

    SharedPtr &operator=(const SharedPtr& shared_ptr) {
        SharedPtr<T>(shared_ptr).swap(*this);
        return *this;
    }

    template<typename Y>
    SharedPtr<T>& operator=(const SharedPtr<Y>& shared_ptr) {
        SharedPtr<T>(shared_ptr).swap(*this);
        return *this;
    }


    template<typename Y>
    SharedPtr<T>& operator=(SharedPtr<Y>&& shared_ptr) {
        SharedPtr<T>(std::move(shared_ptr)).swap(*this);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& shared_ptr) {
        SharedPtr<T>(std::move(shared_ptr)).swap(*this);
        return *this;
    }


    template<typename Y, typename Deleter = std::default_delete<Y>, typename Allocator = std::allocator<Y>>
    SharedPtr(Y* pointer, const Deleter& del, const Allocator& alloc) : ptr(pointer) {
        using ControlPointerAllocator =
                typename std::allocator_traits<Allocator>::template rebind_alloc<blocks::ControlPointerBlock<Y, Deleter, Allocator>>;
        using PointerTraits = std::allocator_traits<ControlPointerAllocator>;
        ControlPointerAllocator pointer_alloc(alloc);
        auto* new_block = PointerTraits::allocate(pointer_alloc, 1);
        if (!std::is_same_v<Allocator, std::allocator<T>>) {
            new(new_block) blocks::ControlPointerBlock<Y, Deleter, Allocator>(pointer, del, alloc);
            block = new_block;
        }
        else {
            try {
                PointerTraits::construct(
                        pointer_alloc, new_block, pointer, del, alloc);
                block = new_block;
                //++(block->count.shared_count);
            } catch (...) {
                const_cast<Deleter &>(del)(pointer);
                throw;
            }
        }


    }

    template<typename Y>
    SharedPtr(Y* pointer): SharedPtr(pointer, std::default_delete<Y>(), std::allocator<Y>()) {}

    template<typename Y, typename Deleter = std::default_delete<Y>>
    SharedPtr(Y* pointer, const Deleter& del): SharedPtr(pointer, del, std::allocator<Y>()) {}


    SharedPtr(const SharedPtr& other) : block(other.block), ptr(other.ptr) {
        if (block){
            ++(block->count.shared_count);
        }
    }

    template<typename Y>
    SharedPtr(const SharedPtr<Y>& other) : block(other.block), ptr(other.ptr) {
        ++(block->count.shared_count);
    }

    ~SharedPtr() {
        if (!block) return;
        if (block->count.shared_count > 0){
            --(block->count.shared_count);
        }
        if (block->count.shared_count == 0) {
            block->delete_object();
            --block->count.weak_count;
            if (block->count.weak_count == 0){
                block->deallocate();
            }
        }

    }

    template<typename Y>
    SharedPtr(SharedPtr<Y>&& other) noexcept: block(std::move(other.block)), ptr(std::move(other.ptr)) {
        other.block = nullptr;
        other.ptr = nullptr;
    }

    SharedPtr(SharedPtr&& other) noexcept: block(std::move(other.block)), ptr(std::move(other.ptr)) {
        other.block = nullptr;
        other.ptr = nullptr;
    }

    T &operator*() const {
        return *ptr;
    }
    T &operator*() {
        return *ptr;
    }


    T *operator->() const {
        return ptr;
    }

    size_t use_count() const {
        return block->count.shared_count;
    }



    void swap(SharedPtr& argument) {
        std::swap(ptr, argument.ptr);
        std::swap(block, argument.block);
    }

    template<typename Y>
    void reset(Y* pointer) {
        SharedPtr<T> new_ptr(pointer);
        swap(new_ptr);
    }

    void reset() {
        SharedPtr().swap(*this);
    }

    T *get() const {
        return ptr;
    }

};

template<typename T, typename ... Args, typename Allocator>
SharedPtr<T> allocateShared(const Allocator& alloc, Args&&... args) {
    using ControlArgsAllocator =
            typename std::allocator_traits<Allocator>::template rebind_alloc<blocks::ControlArgsBlock<T, Allocator>>;
    using ArgsTraits = std::allocator_traits<ControlArgsAllocator>;
    ControlArgsAllocator arg_alloc(alloc);
    auto *new_block = ArgsTraits::allocate(arg_alloc, 1);
    try {
        ArgsTraits::construct(
                arg_alloc, new_block,  alloc, std::forward<Args>(args)...);
        //new blocks::ControlArgsBlock<T, Allocator>(new_block, alloc, std::forward<Args>(args)...);
        return SharedPtr<T>(((new_block->val).value()), new_block);
    } catch (...) {
        ArgsTraits::deallocate(arg_alloc, new_block, 1);
        throw;
    }
}

template<typename T, typename ... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return allocateShared<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

template<typename T>
class WeakPtr {

    template<typename Y>
    friend
    class SharedPtr;

    template<typename Y>
    friend
    class WeakPtr;

private:
    blocks::BaseBlock *base_block = nullptr;

public:

    WeakPtr() = default;

    template<typename Y>
    WeakPtr(const SharedPtr<Y>& argument) noexcept: base_block(static_cast<blocks::BaseBlock*>(argument.block)) {
        if (base_block) {
            ++(base_block->count.weak_count);
        }
    }

    WeakPtr(const WeakPtr& argument) noexcept: base_block(argument.base_block) {
        if (base_block) {
            ++(base_block->count.weak_count);
        }
    }

    template<typename Y>
    WeakPtr(const WeakPtr<Y>& argument) noexcept: base_block(argument.base_block) {
        ++(base_block->count.weak_count);
    }

    WeakPtr(WeakPtr&& argument) noexcept: base_block(std::move(argument.base_block)) {
        argument.base_block = nullptr;
    }

    template<typename Y>
    WeakPtr(WeakPtr<Y>&& argument) noexcept: base_block(std::move(argument.base_block)) {
        argument.base_block = nullptr;
    }

    void swap(WeakPtr& argument){
        std::swap(base_block, argument.base_block);
    }

    bool expired() const noexcept {
        return base_block->count.shared_count == 0;
    }

    template<typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& argument) {
        WeakPtr<T>(argument).swap(*this);
        return *this;
    }

    WeakPtr& operator=(const WeakPtr& argument) {
        WeakPtr<T>(argument).swap(*this);
        return *this;;
    }

    WeakPtr& operator=(const SharedPtr<T>& argument) {
        WeakPtr<T>(argument).swap((*this));
        return *this;
    }

    template<typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& argument) {
        WeakPtr<T>(std::move(argument)).swap(*this);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& argument) {
        WeakPtr<T>(std::move(argument)).swap(*this);
        return *this;
    }

    SharedPtr<T> lock() const{
        return expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
    }

    size_t use_count() const {
        return base_block->count.shared_count;
    }

    ~WeakPtr() {
        if (base_block == nullptr)return;
        else {
            if (base_block->count.weak_count > 0) {
                --(base_block->count.weak_count);
            }
            if (base_block->count.weak_count <= 0 && base_block->count.shared_count <= 0) {
                base_block->deallocate();
                if(!base_block) {
                    --base_block->count.shared_count;
                }
            }
        }
    }
};


