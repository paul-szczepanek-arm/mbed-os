/* mbed Microcontroller Library
 * Copyright (c) 2015-2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_CIRCULARBUFFER_H
#define MBED_CIRCULARBUFFER_H

#include <stdint.h>
#include "platform/mbed_critical.h"
#include "platform/mbed_assert.h"
#include "platform/Span.h"

namespace mbed {

namespace internal {
/* Detect if CounterType of the Circular buffer is of unsigned type. */
template<typename T>
struct is_unsigned {
    static const bool value = false;
};
template<>
struct is_unsigned<unsigned char> {
    static const bool value = true;
};
template<>
struct is_unsigned<unsigned short> {
    static const bool value = true;
};
template<>
struct is_unsigned<unsigned int> {
    static const bool value = true;
};
template<>
struct is_unsigned<unsigned long> {
    static const bool value = true;
};
template<>
struct is_unsigned<unsigned long long> {
    static const bool value = true;
};
}

/** \addtogroup platform-public-api */
/** @{*/
/**
 * \defgroup platform_CircularBuffer CircularBuffer functions
 * @{
 */

/** Templated Circular buffer class
 *
 *  @note Synchronization level: Interrupt safe.
 *  @note CounterType must be unsigned and consistent with BufferSize.
 */
template<typename T, uint32_t BufferSize, typename CounterType = uint32_t>
class CircularBuffer {
public:
    CircularBuffer() : _head(0), _tail(0), _full(false)
    {
        MBED_STATIC_ASSERT(
            internal::is_unsigned<CounterType>::value,
            "CounterType must be unsigned"
        );

        MBED_STATIC_ASSERT(
            (sizeof(CounterType) >= sizeof(uint32_t)) ||
            (BufferSize < (((uint64_t) 1) << (sizeof(CounterType) * 8))),
            "Invalid BufferSize for the CounterType"
        );
    }

    ~CircularBuffer()
    {
    }

    /** Push the transaction to the buffer. This overwrites the buffer if it's full.
     *
     * @param data Data to be pushed to the buffer.
     */
    void push(const T &data)
    {
        core_util_critical_section_enter();
        if (full()) {
            _tail++;
            if (_tail == BufferSize) {
                _tail = 0;
            }
        }
        _buffer[_head++] = data;
        if (_head == BufferSize) {
            _head = 0;
        }

        if (_head == _tail) {
            _full = true;
        }

        core_util_critical_section_exit();
    }

    /** Push the transaction to the buffer. This overwrites the buffer if it's full.
     *
     * @param src Data to be pushed to the buffer.
     * @param len Number of items to be pushed to the buffer.
     */
    void push(const T* src, CounterType len)
    {
        core_util_critical_section_enter();

        /* if we try to write more bytes than the buffer can hold we only bother writing the last bytes */
        if (len > BufferSize) {
            _tail = 0;
            _head = 0;
            _full = true;
            std::copy(src, src + len, _buffer);
        } else {
            /* we need to adjust the tail at the end if we're filling the buffer of overflowing */
            bool adjust_tail = ((BufferSize - non_critical_size()) <= len);

            CounterType written = len;

            /* on first pass we write as much as we can to the right of head */
            if ((_head + len) > BufferSize) {
                written = BufferSize - _head;
            }

            std::copy(src, src + written, _buffer + _head);

            CounterType left_to_write = len - written;
            _head += written;
            _head %= BufferSize;

            /* we might need to continue to write from the start of the buffer */
            if (left_to_write) {
                std::copy(src + written, src + written + left_to_write, _buffer + left_to_write);
                _head = written;
            }

            if (adjust_tail) {
                _tail = _head;
                _full = true;
            }
        }

        core_util_critical_section_exit();
    }

    /** Push the transaction to the buffer. This overwrites the buffer if it's full.
     *
     * @param src Data to be pushed to the buffer.
     */
    template<CounterType N>
    void push(T (&src)[N])
    {
        push(src, N);
    }

    /** Push the transaction to the buffer. This overwrites the buffer if it's full.
     *
     * @param src Data to be pushed to the buffer.
     */
    void push(const mbed::Span<const T>& src)
    {
        push(src.data(), src.size());
    }

    /** Pop from the buffer.
     *
     * @param data Container to store the data to be popped from the buffer.
     * @return True if data popped.
     */
    bool pop(T& data)
    {
        bool data_popped = false;

        core_util_critical_section_enter();

        if (!non_critical_empty()) {
            data_popped = true;

            data = _buffer[_tail];
            _tail = incrementCounter(_tail);
            _full = false;
        }

        core_util_critical_section_exit();

        return data_popped;
    }

    /**
     * Pop multiple elements from the buffer.
     *
     * @param dest The array which will receive the elements.
     * @param len The number of elements to pop.
     *
     * @return The number of elements popped.
     */
    CounterType pop(T* dest, CounterType len)
    {
        CounterType data_popped = 0;

        core_util_critical_section_enter();

        if(!non_critical_empty()) {
            /* make sure we only try to read as much as we have items present */
            if (len > non_critical_size()) {
                len = non_critical_size();
            }
            data_popped = len;

            /* items may be split by overlap, take only the number we have to the right of tail */
            if ((_tail + data_popped) > BufferSize) {
                data_popped = BufferSize - _tail;
            }

            std::copy(_buffer + _tail, _buffer + _tail + data_popped, dest);

            /* if we looped over the end we may need to pop again */
            CounterType left_to_pop = len - data_popped;

            if (left_to_pop) {
                std::copy(_buffer, _buffer + left_to_pop, dest + data_popped);

                data_popped += left_to_pop;
            }

            _tail += data_popped;
            _tail %= BufferSize;
            _full = false;
        }

        core_util_critical_section_exit();

        return data_popped;
    }

    /**
     * Pop multiple elements from the buffer.
     *
     * @param dest The array which will receive the elements.
     *
     * @return The number of elements popped.
     */
    template<CounterType N>
    CounterType pop(T (&dest)[N])
    {
        return pop(dest, N);
    }

    /** Check if the buffer is empty.
     *
     * @return True if the buffer is empty, false if not.
     */
    bool empty() const
    {
        core_util_critical_section_enter();
        bool is_empty = non_critical_empty();
        core_util_critical_section_exit();
        return is_empty;
    }

    /** Check if the buffer is full.
     *
     * @return True if the buffer is full, false if not
     */
    bool full() const
    {
        return _full;
    }

    /**
     * Reset the buffer.
     */
    void reset()
    {
        core_util_critical_section_enter();
        _head = 0;
        _tail = 0;
        _full = false;
        core_util_critical_section_exit();
    }

    /**
     * Get the number of elements currently stored in the circular_buffer.
     */
    CounterType size() const
    {
        core_util_critical_section_enter();
        CounterType elements = non_critical_size();
        core_util_critical_section_exit();
        return elements;
    }

    /** Peek into circular buffer without popping.
     *
     * @param data Data to be peeked from the buffer.
     * @return True if the buffer is not empty and data contains a transaction, false otherwise.
     */
    bool peek(T &data) const
    {
        bool data_updated = false;
        core_util_critical_section_enter();
        if (!empty()) {
            data = _buffer[_tail];
            data_updated = true;
        }
        core_util_critical_section_exit();
        return data_updated;
    }

private:
    bool non_critical_empty() const
    {
        bool is_empty = (_head == _tail) && !_full;
        return is_empty;
    }

    bool non_critical_size() const
    {
        CounterType elements;
        if (!_full) {
            if (_head < _tail) {
                elements = BufferSize + _head - _tail;
            } else {
                elements = _head - _tail;
            }
        } else {
            elements = BufferSize;
        }
        return elements;
    }

    CounterType incrementCounter(CounterType val)
    {
        return (++val) % BufferSize;
    }

private:
    T _buffer[BufferSize];
    CounterType _head;
    CounterType _tail;
    bool _full;
};

/**@}*/

/**@}*/

}

#endif
