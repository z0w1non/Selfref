#include "allocator.h"

#include <cstdlib>
#include <climits>
#include <cstring>
#include <mutex>
#include <vector>
#include <list>
#include <array>
#include <algorithm>
#include <memory>

//////////////////////
// Macro definition //
//////////////////////
constexpr static std::size_t default_memory_pool_size = 0x100000;
constexpr static int N = 5;
constexpr static std::size_t first_level_index_max = sizeof(size_t) * CHAR_BIT - N;
constexpr static std::size_t second_level_index_max = 2 << N;
constexpr static int mask_used_flag = 0x00000001;
constexpr static int mask_marked_flag = 0x00000002;

constexpr static std::size_t block_overhead = sizeof(std::size_t) * 2;
constexpr static std::size_t pool_overhead = sizeof(std::size_t) * 3;
constexpr static std::size_t initial_overhead = pool_overhead + block_overhead;
constexpr static std::size_t min_page_size = 4096 - initial_overhead;

/////////////////////
// Type definition //
/////////////////////

// +---+----------------+-----------+--------------+---------------+---+
// | 0 | FRONT SIZE (N) | USED FLAG | ENTITY (ptr) | BACK SIZE (N) | 0 |
// +---+----------------+-----------+--------------+---------------+---+
class block_t
{
public:
    block_t(void * ptr);
    block_t(const block_t & b) = default;
    block_t & operator =(const block_t & b) = default;
    operator void * () const;

    void *          get_entity();
    void            clear_entity();
    std::size_t &   flag();
    void            set_size(std::size_t size);
    std::size_t     get_size();
    void            set_used_flag(bool used_flag);
    int             get_used_flag();
    void            set_marked_flag(bool marked_flag);
    int             get_marked_flag();
    block_t         prev();
    block_t         next();
    block_t         divide(std::size_t size);
    block_t         join_free_blocks();

private:
    block_t();
    std::size_t & front_size();
    std::size_t & back_size();
    void * ptr;
};

class page_t
{
public:
    page_t(std::size_t size);
    block_t front_memory_block();
    std::size_t get_committed_memory();

private:
    std::unique_ptr<char[]> ptr;
    std::size_t size;
};

class pool_t : public std::list<page_t>
{
public:
    pool_t();
    void commit(std::size_t size);
    void * allocate(std::size_t size);
    void destroy(void * ptr);
    std::size_t get_used_memory();
    std::size_t get_committed_memory();
    void gc_begin();
    void gc_mark(void * ptr);
    void gc_end();

private:
    using free_block_list_t = std::list<block_t>;
    std::array<free_block_list_t, first_level_index_max * second_level_index_max> free_block_lists;
    std::size_t get_first_level_index(std::size_t value);
    std::size_t get_second_level_index(std::size_t value, std::size_t msb);
    free_block_list_t & get_free_block_list(std::size_t first_level_index, std::size_t second_level_index);
    void add_free_block(block_t ptr);
    void * allocate_internal(std::size_t size);

    template<typename function_type>
    void for_each(function_type callback);

    std::size_t used_memory;
    std::size_t committed_memory;
};

class gc_allocator::impl_t {
public:
    pool_t pool;
};

/////////////
// Utility //
/////////////
void * offset(void * ptr, std::ptrdiff_t offset);

page_t::page_t(std::size_t size)
{
    this->size = size + initial_overhead;
    ptr.reset(new char[this->size]);

    *(std::size_t *)offset(ptr.get(), 0) = 0;
    *(std::size_t *)offset(ptr.get(), this->size - sizeof(std::size_t)) = 0;

    block_t front = front_memory_block();
    front.set_size(size);
    front.set_used_flag(true);
    front.set_marked_flag(false);
    memset(front, 0, size);
}

block_t page_t::front_memory_block()
{
    return offset(ptr.get(), sizeof(std::size_t) * 3);
}

std::size_t page_t::get_committed_memory()
{
    return size;
}

pool_t::pool_t()
    : used_memory{ 0 }
    , committed_memory{ 0 }
{
    commit(default_memory_pool_size);
}

void pool_t::commit(std::size_t size)
{
    size = std::max(min_page_size, size);
    push_back(size);
    add_free_block(back().front_memory_block());
    committed_memory += back().get_committed_memory();
}

void * pool_t::allocate(std::size_t size)
{
    void * ptr = allocate_internal(size);
    if (ptr)
        return ptr;
    commit(committed_memory);
    ptr = allocate_internal(size);
    return ptr;
}

void * pool_t::allocate_internal(std::size_t size)
{
    std::size_t first_level_index = get_first_level_index(size);
    std::size_t second_level_index = get_second_level_index(size, first_level_index);
    for (std::size_t i = first_level_index; i < first_level_index_max; ++i)
    {
        for (std::size_t j = first_level_index; j < first_level_index_max; ++j)
        {
            free_block_list_t & list = get_free_block_list(i, j);
            if (!list.empty())
            {
                block_t allocated_mamory_block = list.front();
                list.pop_front();
                block_t rest_memory_block = allocated_mamory_block.divide(size);
                used_memory += allocated_mamory_block.get_size();
                if (rest_memory_block)
                    add_free_block(rest_memory_block);
                return allocated_mamory_block;
            }
        }
    }
    return nullptr;
}

void pool_t::destroy(void * ptr)
{
    block_t b = ptr;
    if (b.get_used_flag())
    {
        used_memory -= b.get_size();
        b = b.join_free_blocks();
        add_free_block(b);
        b.set_used_flag(false);
        b.set_marked_flag(false);
    }
}

std::size_t pool_t::get_used_memory()
{
    return used_memory;
}


std::size_t pool_t::get_committed_memory()
{
    return committed_memory;
}

std::size_t pool_t::get_first_level_index(std::size_t value)
{
    std::size_t i;
    for (i = sizeof(size_t) * CHAR_BIT - 1; (value & ((std::size_t)1 << i)) == 0; --i);
    return i;
}

std::size_t pool_t::get_second_level_index(std::size_t value, std::size_t msb)
{
    return (value & ((static_cast<std::size_t>(1) << msb) ^ msb)) >> (msb - N);
}

pool_t::free_block_list_t & pool_t::get_free_block_list(std::size_t first_level_index, std::size_t second_level_index)
{
    return free_block_lists[first_level_index_max * first_level_index + second_level_index];
}

void pool_t::add_free_block(block_t memory_block)
{
    std::size_t first_level_index = get_first_level_index(memory_block.get_size());
    std::size_t second_level_index = get_second_level_index(memory_block.get_size(), first_level_index);
    free_block_list_t & list = get_free_block_list(first_level_index, second_level_index);
    list.push_back(memory_block);
}

void * offset(void * ptr, std::ptrdiff_t offset)
{
    return reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + offset);
}

block_t::block_t(void * ptr)
    : ptr{ ptr }
{
}

block_t::operator void * () const
{
    return ptr;
}

std::size_t & block_t::front_size()
{
    return *(std::size_t *)offset(ptr, sizeof(std::size_t) * -2);
}

std::size_t & block_t::back_size()
{
    return *(std::size_t *)offset(ptr, front_size());
}

void * block_t::get_entity()
{
    return offset(ptr, sizeof(std::size_t));
}

void block_t::clear_entity()
{
    memset(get_entity(), 0, get_size());
}

std::size_t & block_t::flag()
{
    return *(std::size_t *)offset(ptr, sizeof(std::size_t) * -1);
}

void block_t::set_size(std::size_t size)
{
    front_size() = size;
    back_size() = size;
}

std::size_t block_t::get_size()
{
    return front_size();
}

void block_t::set_used_flag(bool used_flag)
{
    if (used_flag)
        flag() |= mask_used_flag;
    else
        flag() &= ~mask_used_flag;
}

int block_t::get_used_flag()
{
    return (flag() | mask_used_flag) != 0;
}

void block_t::set_marked_flag(bool marked_flag)
{
    if (marked_flag)
        flag() |= mask_marked_flag;
    else
        flag() &= ~mask_marked_flag;
}

int block_t::get_marked_flag()
{
    return (flag() | mask_marked_flag) != 0;
}

block_t block_t::prev()
{
    std::size_t * prev_block_back_size;
    prev_block_back_size = (std::size_t *)offset(ptr, sizeof(std::size_t) * -2);
    if (ptr && *prev_block_back_size)
        return offset(ptr, -(ptrdiff_t)*prev_block_back_size - sizeof(std::size_t));
    return nullptr;
}

block_t block_t::next()
{
    std::size_t * next_block_front_size;
    next_block_front_size = (std::size_t *)offset(&back_size(), sizeof(std::size_t));
    if (ptr && *next_block_front_size)
        return offset(next_block_front_size, sizeof(std::size_t));
    return nullptr;
}

block_t block_t::divide(std::size_t size)
{
    std::size_t old_size, new_size;
    if (front_size() < size - block_overhead)
        return nullptr;

    old_size = front_size();
    new_size = front_size() - size - block_overhead;
    front_size() = size;
    back_size() = size;
    *(std::size_t *)offset(&back_size(), sizeof(std::size_t)) = new_size;
    *(std::size_t *)offset(&back_size(), sizeof(std::size_t) + new_size) = new_size;
    return next();
}

block_t block_t::join_free_blocks()
{
    std::size_t total_size;

    block_t prev_block = prev();
    block_t next_block = next();

    if (prev_block && !prev_block.get_used_flag())
    {
        total_size = get_size() + prev_block.get_size() + block_overhead;
        prev_block.set_size(total_size);
        ptr = prev_block.ptr;
    }

    if (next_block && !next_block.get_used_flag())
    {
        total_size = get_size() + next_block.get_size() + block_overhead;
        set_size(total_size);
    }

    return *this;
}

template<typename function_type>
void pool_t::for_each(function_type callback)
{
    for (page_t & page : *this)
        for (block_t b = page.front_memory_block(); b; b = b.next())
            callback(b);
}

gc_allocator::gc_allocator()
{
    impl = std::make_unique<impl_t>();
}

gc_allocator::~gc_allocator()
{
}

void * gc_allocator::allocate(gc_allocator::size_type size)
{
    return impl->pool.allocate(size);
}

void gc_allocator::deallocate(gc_allocator::pointer ptr)
{
    impl->pool.destroy(ptr);
}

void pool_t::gc_begin()
{
    for_each([](block_t block) {
        block.set_marked_flag(0);
    });
}

void gc_allocator::gc_begin()
{
    static std::mutex mtx;
    std::lock_guard<decltype(mtx)> lock{ mtx };

    return impl->pool.gc_begin();
}

void pool_t::gc_mark(void * ptr)
{
    block_t{ ptr }.set_marked_flag(1);
}

void gc_allocator::gc_mark(gc_allocator::pointer ptr)
{
    static std::mutex mtx;
    std::lock_guard<decltype(mtx)> lock{ mtx };

    return impl->pool.gc_mark(ptr);
}

void pool_t::gc_end()
{
    for_each([this](block_t block) {
        if (!block.get_marked_flag())
            this->destroy(block);
    });
}

void gc_allocator::gc_end()
{
    static std::mutex mtx;
    std::lock_guard<decltype(mtx)> lock{ mtx };

    return impl->pool.gc_end();
}

void gc_begin_callback(block_t memory_block)
{
    memory_block.set_marked_flag(0);
}

gc_allocator::size_type gc_allocator::get_used_memory()
{
    return impl->pool.get_used_memory();
}

gc_allocator::size_type gc_allocator::get_committed_memory()
{
    return impl->pool.get_committed_memory();
}
