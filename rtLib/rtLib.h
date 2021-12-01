//
// Created by cxworks on 19-5-26.
//

#ifndef G_PARSER_RTLIB_H
#define G_PARSER_RTLIB_H

#define BOTTOM_MAP_STLMAP_IMPL
#define UNIVERSE_SET_SPARSEBITVECTOR_IMPL
#define BOTTOM_SET_SPARSESET_IMPL

#include <iostream>

#include <pthread.h>
#include "PageMapUnsync.h"
#include "PageMap.h"
#include "PageMapUnsyncOffset.h"
#include <map>

#ifdef BOTTOM_SET_STLSET_IMPL
#include <unordered_set>


using namespace std;
#endif

#ifdef BOTTOM_SET_SPARSESET_IMPL

#include "util_bitmap_sparse_bitmap.h"

#endif

using namespace util;
typedef unsigned long long ull;
namespace bottom {
    template<class E>
    class set;
}

namespace universe {
    constexpr uint lsh(uint l1, uint l2){
        return l1 << l2;
    }
    template<class K, class V, bool sync = false, uint shift = 0, uint ratio = sizeof(V),
            bool offset = (ratio <= 3)>
    class map {
    private:
        sync_p::PageMap<sync_p::PageSizes<20, 16, 12 - shift>, K, V> sync_pagemap;
        unsync::PageMap<unsync::PageSizes<20, 16, 12 - shift>, K, V> unsync_pagemap;
        offset::PageMap<K, V, shift, ratio> offset_pagemap;
    public:
        static void init() {
            if (offset)
                offset::PageMap<K, V, shift>::init();
        }

        map();

        bool find(K k);

        unsigned long size();

        V &get(K k);

        inline ull get(K k, size_t size);

        inline void set(K k, ull v, size_t size);

        char &get1(K k);

         short &get2(K k);

         int &get4(K k);

         long long &get8(K k);

        V &getOrDefault(K k, V def);

        V &operator[](K k);

        void add_range(K k, V v, size_t size);

        void remove_range(K k, size_t size);

        void add_range(void *st, ull v, size_t size, uint off, uint unit_size);

        void remove_range(void *k, size_t size, uint off, uint unit_size);

        V or_range(K k, size_t size);

        void cpy_range(K k1, K k2, size_t size);

        void move_range(K k1, K k2, size_t size);

        bool test(K key);
    };


    template<class E>
    class set {
        friend bottom::set<E>;
    private:
#ifdef UNIVERSE_SET_SPARSEBITVECTOR_IMPL
        SparseBitmap<E> universe_sparseset;
        int universe_id = 0;
#endif
    public:
        set();

        explicit set(E e);

        set(const set<E> &e);

        bool find(E e);

        void add(E e);

        void remove(E e);

        bool empty();

        set<E> operator+(set<E> &another);

        set<E> operator&(set<E> &another);

        set<E> operator|(set<E> &another);

        set<E> &operator+=(set<E> &another);

        set<E> &operator|=(set<E> &another);

        set<E> &operator&=(set<E> &another);


        set<E> operator+(bottom::set<E> &another);

        set<E> operator-(bottom::set<E> &another);

        bottom::set<E> operator&(bottom::set<E> &another);

        set<E> operator|(bottom::set<E> &another);

        set<E> &operator+=(bottom::set<E> &another);

        set<E> &operator-=(bottom::set<E> &another);

        set<E> &operator|=(bottom::set<E> &another);
    };
}


namespace bottom {
    template<class K, class V, bool sync>
    class map {
    private:


#ifdef BOTTOM_MAP_STLMAP_IMPL
        std::map<K, V> bottom_stlmap;
        int bottom_id = 0;
        pthread_spinlock_t l;
#endif
//        pthread_mutex_t instance_lock;
    public:
        map();

        bool find(K k);

        unsigned long size();

        V &get(K k);

        V &operator[](K k);

        void remove(K k);

        void add_range(K k, V v, int size);

        void remove_range(K k, int size);

    };

    template<class E>
    class set {
        friend universe::set<E>;
    private:
#ifdef BOTTOM_SET_STLSET_IMPL
        unordered_set<E> bottom_set;
        int bottom_id = 0;
#endif

#ifdef BOTTOM_SET_SPARSESET_IMPL
        SparseBitmap<E> bottom_set;
        int bottom_id = 1;
#endif
    public:
        set();

        explicit set(E e);

        set(const set<E> &e);

        bool find(E e);

        void add(E e);

        void remove(E e);

        unsigned long size();

        auto begin();

        auto end();

        set<E> operator+(set<E> &another);

        set<E> operator-(set<E> &another);

        set<E> operator&(set<E> &another);

        set<E> operator|(set<E> &another);

        set<E> &operator+=(set<E> &another);

        set<E> &operator-=(set<E> &another);

        set<E> operator&=(set<E> &another);

        set<E> &operator|=(set<E> &another);

        universe::set<E> operator+(universe::set<E> &another);

        set<E> operator-(universe::set<E> &another);

        set<E> operator&(universe::set<E> &another);

        universe::set<E> operator|(universe::set<E> &another);

        set<E> &operator-=(universe::set<E> &another);

        set<E> &operator&=(universe::set<E> &another);


    };


}

namespace rt_lib {
    template<class K, class V, bool uni = false, bool sync = false, int shift = 0>
    class map {
    private:
        universe::map<K, V, sync, shift> universe_map;
        bottom::map<K, V, sync> bottom_map;
    public:
        static void init() {

            if (uni) {
                universe::map<K, V, sync, shift>::init();
            }
        }

        map();

        inline bool find(K k);

        inline unsigned long size();

        inline V &get(K k);

        inline  ull get(K k, size_t size);

        inline void set(K k, ull v, size_t size);

        unsigned char &get1(K k);

        unsigned short &get2(K k);

        unsigned int &get4(K k);

        unsigned long long &get8(K k);

        inline V &getOrDefault(K k, V def);

        inline V &operator[](K k);

        inline void remove(K k);

        inline void remove_range(K k, size_t size);

        inline void remove_range(void *k, size_t size, uint offset, uint unit_size);

        inline void remove_range(long long k, size_t size, uint offset, uint unit_size){
            this->remove_range((void*)k, size, offset, unit_size);
        }

        inline void add_range(K k, V v, size_t size);

        inline void add_range(K k, ull v, size_t size, uint offset, uint unit_size){
            this->add_range((void*)k, v, size, offset, unit_size);
        }

        inline void set_range(K k, ull v, size_t size, uint offset, uint unit_size){
            this->add_range((void*)k, v, size, offset, unit_size);
        }

        inline void add_range(void *k, ull v, size_t size, uint offset, uint unit_size);

        inline bool check_range(K k, size_t size, V v);

        inline bool check_range(K k, size_t size, u_char v, uint off, uint unit_size){
            return this->check_range((void*)k, size, v, off, unit_size);
        }

        inline bool check_range(void *k, size_t size, u_char v, uint off, uint unit_size);

        inline void cpy_range(K k1, K k2, size_t size);

        inline void move_range(K k1, K k2, size_t size);

        inline V or_range(K k, size_t size);

        inline bool find(void *k) {
            return this->find(((K) k));
        }

        inline V &getOrDefault(void *k, V def) {
            return this->getOrDefault((K) k, def);
        }

        inline V &get(void *k) {
            return this->get((K) k);
        }

        inline V &get(int k) {
            return this->get((K) k);
        }

        inline ull get(void *k, size_t size) {
            return this->get((K) k, size);
        }

        inline void set(void *k, ull v, size_t size) {
            this->set((K) k, v, size);
        }

        inline  char &get1(void *k) {
            return this->get1((K) k);
        }

        inline  short &get2(void *k) {
            return this->get2((K) k);
        }

        inline  int &get4(void *k) {
            return this->get4((K) k);
        }

        inline  long long &get8(void *k) {
            return this->get8((K) k);
        }

        inline V &operator[](void *k) {
            return this->get((K) k);
        }

        inline V &operator[](int k) {
            return this->get((K) k);
        }

        inline void remove(void *k) {
            this->remove((K) k);
        }

        inline void remove_range(void *k, int size) {
            this->remove_range((K) k, size);
        }

        inline void add_range(void *k, V v, size_t size) {
            this->add_range((K) k, v, size);
        }

        inline void cpy_range(void *k1, void *k2, int size) {
            this->cpy_range((K) k1, (K) k2, size);
        }

        inline void move_range(void *k1, void *k2, int size) {
            this->move_range((K) k1, (K) k2, size);
        }

        inline bool check_range(void *k, int size, V v) {
            return this->check_range((K) k, size, v);
        }

        inline V or_range(void *k, int size) {
            return this->or_range((K) k, size);
        }

    };

    template<class E>
    class set {
    private:
        bool universe = false;
        universe::set<E> universe_set;
        bottom::set<E> bottom_set;
    public:
        set();

        explicit set(bool);

        bool find(E e);

        void add(E e);

        void remove(E e);

        bool empty();

        size_t size();

        set<E> operator+(set<E> &another);

        set<E> operator-(set<E> &another);

        set<E> operator&(set<E> &another);

        set<E> operator|(set<E> &another);

        set<E> &operator+=(set<E> &another);

        set<E> &operator-=(set<E> &another);

        set<E> &operator&=(set<E> &another);

        set<E> &operator|=(set<E> &another);

        void remove_range(E e, int size);

        void add_range(E e, int size);

        universe::set<E> &getUniverse() {
            return universe_set;
        }

        bottom::set<E> &getBottom() {
            return bottom_set;
        }

    };


}


#endif //G_PARSER_RTLIB_H
