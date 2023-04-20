
#ifndef MEM_ARENA_HH__
#define MEM_ARENA_HH__

#include <stddef.h>
#include <stdint.h>

namespace mem
{
template <size_t mem_size_> class arena
{
  protected:
    uint8_t mem_[mem_size_];
    size_t index_;
    size_t allocs_;

  public:
    void*
    alloc(size_t size)
    {
        if (index_ + size > mem_size_) {
            return nullptr;
        }

        void* mem = mem_ + index_;
        index_ += size;
        allocs_++;

        return mem;
    }

    void
    flush()
    {
        index_ = 0;
        allocs_ = 0;
    }

    void
    dealloc(void* ptr)
    {
        if (allocs_) {
            allocs_--;
        }

        if (!allocs_) {
            this->flush();
        }
    }
};
}; // namespace mem

#endif // MEM_ARENA_HH__