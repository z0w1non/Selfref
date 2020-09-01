#include "allocator.h"

#include <cstdlib>
#include <climits>
#include <cstring>
#include <mutex>
#include <vector>
#include <list>
#include <array>
#include <algorithm>

//////////////////////
// Macro definition //
//////////////////////
constexpr static std::size_t default_memory_pool_size = 0x100000;
constexpr static int N = 5;
constexpr static std::size_t first_level_index_max = sizeof(size_t) * CHAR_BIT - N;
constexpr static std::size_t second_level_index_max = 2 << N;
constexpr static std::size_t total_tag_size = sizeof(size_t) * 2 + sizeof(int);
constexpr static int mask_used_flag = 0x00000001;
constexpr static int mask_marked_flag = 0x00000002;

constexpr static std::size_t initial_overhead = sizeof(std::size_t) * 2 + total_tag_size;
constexpr static std::size_t min_page_size = 4096;

/////////////////////
// Type definition //
/////////////////////

//   sizeof(size_t)   sizeof(size_t)   sizeof(iny)   N        sizeof(size_t)   sizeof(size_t)
// +----------------+----------------+-------------+--------+----------------+----------------+
// | 0              | FRONT SIZE (N) | USED FLAG   | ENTITY | BACK SIZE (N)  | 0              |
// +----------------+----------------+-------------+--------+----------------+----------------+
class memory_block_t
{
public:
    memory_block_t();
    memory_block_t(void * ptr);
    memory_block_t & operator =(const memory_block_t & mb) = default;
    operator void * () const;

    int *           flag();
    void            set_size(size_t size);
    size_t          get_size();
    void            set_used_flag(int used_flag);
    int             get_used_flag();
    void            set_marked_flag(int marked_flag);
    int             get_marked_flag();
    memory_block_t  prev();
    memory_block_t  next();
    memory_block_t  divide(size_t size);
    memory_block_t  join_free_blocks();
    void            for_each(void (*callback)(memory_block_t));

    static void *   allocate(size_t size);
    static void     destroy(void * ptr);

private:
    size_t * front_size();
    size_t * back_size();
    void * ptr;
};

/////////////////////
// Global variable //
/////////////////////

typedef std::vector<memory_block_t> memory_pool_t;
memory_pool_t memory_pool;

typedef std::list<memory_block_t> free_block_list_t;
std::array<free_block_list_t, first_level_index_max * second_level_index_max> free_block_lists;

free_block_list_t & get_free_block_list(size_t first_level_index, size_t second_level_index);
int add_free_block(memory_block_t ptr);
void * offset(void * ptr, std::ptrdiff_t offset);

/////////////////////////////
// GC function declaration //
/////////////////////////////
void allocator_gc_begin();
void allocator_gc_mark(void * ptr);
void allocator_gc_end();
void allocator_gc_begin_callback(memory_block_t);
void allocator_gc_end_callback(memory_block_t);

std::mutex allocator_init_mtx;
int allocator_init_impl()
{
    std::lock_guard<decltype(allocator_init_mtx)> lock{ allocator_init_mtx };

    int i;
    void * ptr = memory_block_t::allocate(default_memory_pool_size);
    if (!ptr)
        goto error;

    memory_pool.push_back(ptr);
    add_free_block(ptr);

    return 0;

error:
    return 1;
}

extern "C" int allocator_init()
{
    return allocator_init_impl();
}

std::mutex allocator_cleanup_mtx;
int allocator_cleanup_impl()
{
    std::lock_guard<decltype(allocator_cleanup_mtx)> lock{ allocator_cleanup_mtx };

    for (auto ptr : memory_pool)
        memory_block_t::destroy(ptr);
    memory_pool.clear();
    return 0;
}

extern "C" int allocator_cleanup()
{
    return allocator_cleanup_impl();
}

size_t get_first_level_index(size_t value)
{
    int i;
    for (i = sizeof(size_t) * CHAR_BIT - 1; (value & ((size_t)1 << i)) == 0; --i);
    return i;
}

size_t get_second_level_index(size_t value, int msb)
{
    return (value & ((1 << msb) ^ msb)) >> (msb - N);
}

free_block_list_t & get_free_block_list(size_t first_level_index, size_t second_level_index)
{
    return free_block_lists[first_level_index_max * first_level_index + second_level_index];
}

int add_free_block(memory_block_t memory_block)
{
    auto first_level_index = get_first_level_index(memory_block.get_size());
    auto second_level_index = get_second_level_index(memory_block.get_size(), first_level_index);
    auto & list = get_free_block_list(first_level_index, second_level_index);
    list.push_back(memory_block);
    return 0;
}

void * offset(void * ptr, std::ptrdiff_t offset)
{
    return (void *)((char *)ptr + offset);
}

memory_block_t::memory_block_t()
    : ptr{}
{
}

memory_block_t::memory_block_t(void * ptr)
    : ptr{ptr}
{
}

memory_block_t::operator void * () const
{
    return ptr;
}

size_t * memory_block_t::front_size()
{
    return (size_t *)offset(ptr, sizeof(size_t) * -1 + sizeof(int) * -1);
}

size_t * memory_block_t::back_size()
{
    return (size_t *)offset(ptr, *front_size());
}

int * memory_block_t::flag()
{
    return (int *)offset(ptr, sizeof(int) * -1);
}

void memory_block_t::set_size(size_t size)
{
    *front_size() = size;
    *back_size() = size;
}

size_t memory_block_t::get_size()
{
    return *front_size();
}

void memory_block_t::set_used_flag(int used_flag)
{
    if (used_flag)
        *flag() |= mask_used_flag;
    else
        *flag() &= ~mask_used_flag;
}

int memory_block_t::get_used_flag()
{
    return (*flag() | mask_used_flag) != 0;
}

void memory_block_t::set_marked_flag(int marked_flag)
{
    if (marked_flag)
        *flag() |= mask_marked_flag;
    else
        *flag() &= ~mask_marked_flag;
}

int memory_block_t::get_marked_flag()
{
    return (*flag() | mask_marked_flag) != 0;
}

void * memory_block_t::allocate(size_t size)
{
    void * temp;
    size_t total_size;

    total_size = size + total_tag_size + sizeof(size_t) * 2;
    temp = malloc(total_size);
    if (!temp)
        return NULL;

    *(size_t *)offset(temp, 0) = 0;
    *(size_t *)offset(temp, total_size - sizeof(size_t)) = 0;

    memory_block_t mb = offset(temp, sizeof(size_t) * 2 + sizeof(int));
    mb.set_size(size);
    mb.set_used_flag(1);
    mb.set_marked_flag(0);
    memset(mb, 0, size);

    return mb;
}

void memory_block_t::destroy(void * ptr)
{
    if (ptr)
        free(offset(ptr, sizeof(size_t) * -2 + sizeof(int) * -1));
}

memory_block_t memory_block_t::prev()
{
    size_t * prev_back_size_tag;
    prev_back_size_tag = (size_t *)offset(ptr, sizeof(size_t) * -2);
    if (ptr && *prev_back_size_tag)
        return offset(ptr, -(ptrdiff_t)*prev_back_size_tag - sizeof(size_t));
    return NULL;
}

memory_block_t memory_block_t::next()
{
    size_t * next_front_size_tag;
    next_front_size_tag = (size_t *)offset(back_size(), sizeof(size_t));
    if (ptr && *next_front_size_tag)
        return offset(next_front_size_tag, sizeof(size_t));
    return NULL;
}

memory_block_t memory_block_t::divide(size_t size)
{
    size_t old_size, new_size;
    if (*front_size() < size - total_tag_size)
        return NULL;
    
    old_size = *front_size();
    new_size = *front_size() - size - total_tag_size;
    *front_size() = size;
    *back_size() = size;
    *(size_t *)offset(back_size(), sizeof(size_t)) = new_size;
    *(size_t *)offset(back_size(), sizeof(size_t) + new_size) = new_size;
    return next();
}

memory_block_t memory_block_t::join_free_blocks()
{
    memory_block_t prev_block, next_block;
    size_t total_size;

    prev_block = prev();
    next_block = next();

    if (prev_block && !prev_block.get_used_flag())
    {
        total_size = get_size() + prev_block.get_size() + total_tag_size;
        prev_block.set_size(total_size);
        ptr = prev_block.ptr;
    }

    if (next_block && !next_block.get_used_flag())
    {
        total_size = get_size() + next_block.get_size() + total_tag_size;
        set_size(total_size);
    }

    return *this;
}

void memory_block_t::for_each(void (*callback)(memory_block_t))
{
    memory_block_t mb = ptr;
    while (mb)
    {
        callback(mb);
        mb = mb.next();
    }
}

extern "C" void * allocate(size_t bytes)
{
    size_t first_level_index, second_level_index, i, j;
    void * ptr;

    if (bytes == 0)
        return NULL;

    first_level_index = get_first_level_index(bytes);
    second_level_index = get_second_level_index(bytes, first_level_index);

    for (i = first_level_index; i < first_level_index_max; ++i)
    {
        for (j = first_level_index; j < first_level_index_max; ++j)
        {
            auto & list = get_free_block_list(i, j);
            if (!list.empty())
            {
                ptr = list.front();
                list.pop_front();
                return ptr;
            }
        }
    }

    memory_block_t mb = memory_block_t::allocate(std::max(bytes, min_page_size));
    memory_pool.push_back(mb);
    auto rest = mb.divide(bytes);
    add_free_block(rest);
    return (void *)mb;
}

extern "C" void destroy(void * ptr)
{
    memory_block_t mb = ptr;

    if (mb.get_used_flag())
    {
        mb = mb.join_free_blocks();
        add_free_block(mb);
        mb.set_used_flag(0);
        mb.set_marked_flag(0);
    }
}

std::mutex allocator_gc_begin_mtx;
void allocator_gc_begin_impl()
{
    std::lock_guard<decltype(allocator_gc_begin_mtx)> lock{ allocator_gc_begin_mtx };
    for (auto & mb : memory_pool)
        mb.for_each(allocator_gc_begin_callback);
}

extern "C" void allocator_gc_begin()
{
    return allocator_gc_begin_impl();
}

std::mutex allocator_gc_mark_mtx;
void allocator_gc_mark_impl(void * ptr)
{
    std::lock_guard<decltype(allocator_gc_mark_mtx)> lock{ allocator_gc_mark_mtx };
    memory_block_t{ptr}.set_marked_flag(1);
}

extern "C" void allocator_gc_mark(void * ptr)
{
    return allocator_gc_mark_impl(ptr);
}

std::mutex allocator_gc_end_mtx;
void allocator_gc_end_impl()
{
    std::lock_guard<decltype(allocator_gc_end_mtx)> lock{ allocator_gc_end_mtx };
    for (auto & mb : memory_pool)
        mb.for_each(allocator_gc_end_callback);
}

extern "C" void allocator_gc_end()
{
    return allocator_gc_end_impl();
}

void allocator_gc_begin_callback(memory_block_t memory_block)
{
    memory_block.set_marked_flag(0);
}

void allocator_gc_end_callback(memory_block_t memory_block)
{
    if (!memory_block.get_marked_flag())
        destroy(memory_block);
}
