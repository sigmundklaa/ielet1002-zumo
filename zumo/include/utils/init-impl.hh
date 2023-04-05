/* TODO: .cc file */

#include "init.hh"

namespace utils
{

template <typename T> int init_guard<T>::nifty_ = 0;

template <typename T>
uint8_t init_guard<T>::mem_[aligned__(sizeof(T), alignof__(T))];

template <typename T> init_guard<T>::init_guard(init_guard::constructor c)
{
    if (init_guard<T>::nifty_++ != 0) {
        return;
    }

    c(reinterpret_cast<T&>(init_guard<T>::mem_));
}

}; // namespace utils