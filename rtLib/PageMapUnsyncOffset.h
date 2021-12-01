/*
 * Copyright (C) 2020 cxworks
 */

#ifndef UTIL_OFFSET_DATATYPES_PAGEMAP_H_
#define UTIL_OFFSET_DATATYPES_PAGEMAP_H_

#include <cassert>
#include <cstdlib>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

namespace offset {

    template<typename Key = uintptr_t, class T = size_t,
            uint shift = 0, uint ratio = sizeof(T), uint size_of_t = sizeof(T)>
    class PageMap {
        //{{{
    public:
//        static_assert(std::is_integral<Key>::value, "Integral key type required");

        typedef Key key_type;
        typedef Key &key_reference;
        typedef const Key &const_key_reference;

        typedef T value_type;
        typedef T &reference;
        typedef T *pointer;
        typedef const T *const_pointer;

        inline T *mem2shadow(key_type key) {
//            assert(key <= begin || key >= end);
            return (T *) ((key ^ middle_small) * ratio - middle * (ratio - 1));
        }


        std::pair<bool, pointer> test(const_key_reference key) {
            return std::make_pair(true, mem2shadow(key));
        }

        std::pair<bool, const_pointer> &test(const_key_reference key) const {
            return std::make_pair(true, mem2shadow(key));
        }

        inline  char &get1(const_key_reference key) {
            return *( char *) mem2shadow(key);
        }

        inline  short &get2(const_key_reference key) {
            return *( short *) mem2shadow(key);
        }

        inline  int &get4(const_key_reference key) {
            return *( int *) mem2shadow(key);
        }

        inline  long long &get8(const_key_reference key) {
            return *( long long *) mem2shadow(key);
        }

        inline reference get(const_key_reference key) {
            return *mem2shadow(key);
        }

        inline value_type or_range(key_type k, uint size) {
            T v = 0;
            for (int i = 0; i < size; i++) {
                T p = *(mem2shadow((key_type) ((char *) k + i)));
                v |= p;
            }
            return v;
        }

        inline void add_range(const_key_reference k, reference v, size_t size) {
//            if (size_of_t == 1)
//                memset(mem2shadow(k), v, size);
//            else {
//                T *p = (T *) k;
//                for (int i = 0; i < size; i++) {
//                    *mem2shadow((key_type) (p + i)) = v;
//                }
//            }

            std::fill_n(mem2shadow(k), size, v);
        }

        inline void cpy_range(const_key_reference k1, const_key_reference k2, size_t n) {
            memcpy(mem2shadow(k1), mem2shadow(k2), n);
        }

        inline void move_range(const_key_reference k1, const_key_reference k2, size_t n) {
            memmove(mem2shadow(k1), mem2shadow(k2), n);
        }

        void add(const_key_reference k, reference v) {
            this->get(k) = v;
            size++;
        }

        void remove(const_key_reference k) {
            value_type init;
            this->get(k) = init;
            size--;
        }

        unsigned long get_size() {
            return size;
        }

        static void init() {
            void *f = mmap((void *) (middle - ratio * begin),
                           sizeof(unsigned long long) * 1024 * 1024 * 1024 * 1024 * 4 * ratio, PROT_READ | PROT_WRITE,
                           MAP_ANON | MAP_PRIVATE | MAP_NORESERVE | MAP_FIXED,
                           -1, 0);
            assert((unsigned long long) f == (middle - ratio * begin));
        }


    private:
        static const unsigned long long begin = 0x100000000000ULL;
        static const unsigned long long middle_small = 0x300000000000ULL;
        static const unsigned long long middle = 0x400000000000ULL;
        static const unsigned long long end = 0x700000000000ULL;
        unsigned long size = 0;
        //}}}
    };


}  // namespace util

#endif  // UTIL_DATATYPES_PAGEMAP_H_
