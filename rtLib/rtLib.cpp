//
// Created by cxworks on 19-6-12.
//

#include "rtLib.h"

namespace universe {


    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    map<K, V, sync, shift, memory_opt, ratio, offset>::map() {

    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    bool map<K, V, sync, shift, memory_opt, ratio, offset>::find(K key) {
        unsigned long long k = (key >> (shift));
        if (offset)
            return true;
        else {
            if (sync)
                return sync_pagemap.test(k).first;
            else
                return unsync_pagemap.test(k).first;
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    unsigned long map<K, V, sync,  shift, memory_opt, ratio, offset>::size() {
        if (offset)
            return offset_pagemap.get_size();
        else {
            if (sync)
                return sync_pagemap.get_size();
            else
                return unsync_pagemap.get_size();
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    V &map<K, V, sync, shift, memory_opt, ratio, offset>::get(K key) {
        unsigned long long k = (key >> (shift));
        if (offset) {
            k = k<<shift;
            return offset_pagemap.get(k);
        }
        else {
            if (sync)
                return sync_pagemap.get(k);
            else
                return unsync_pagemap.get(k);
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    bool map<K, V, sync, shift, memory_opt, ratio, offset>::test(K key) {
        unsigned long long k = (key >> (shift));
        if (offset) {
            return true;
        }
        else {
            if (sync)
                return sync_pagemap.test(k).first;
            else
                return unsync_pagemap.test(k).first;
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    ull map<K, V, sync, shift, memory_opt, ratio, offset>::get(K key, size_t size) {
        unsigned long long k = (key >> (shift));
        if (offset) {
            k = k<<shift;
            switch (size) {
                case 1:
                    return offset_pagemap.get1(k);
                case 2:
                    return offset_pagemap.get2(k);
                case 4:
                    return offset_pagemap.get4(k);
                case 8:
                    return offset_pagemap.get8(k);
                default:
                    size_t shif = 1<<shift;
                    return (ull)offset_pagemap.or_range(k, (size + shif - 1)/shif * shif);
            }
        }else {
            throw "Not Supported!";
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    void map<K, V, sync, shift, memory_opt, ratio, offset>::set(K key, ull v, size_t size) {
        unsigned long long k = (key >> (shift));

        if (offset) {
            k = k<<shift;
            switch (size) {
                case 1:
                    this->get1(k) = v;
                    break;
                case 2:
                    this->get2(k) = v;
                    break;
                case 4:
                    this->get4(k) = v;
                    break;
                case 8:
                    this->get8(k) = v;
                    break;
                default:
                    size_t shif = 1<<shift;
                    offset_pagemap.add_range(k, (V&)v, (size + shif - 1)/shif * shif);
            }
        }else {
            throw "Not Supported!";
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    char &map<K, V, sync, shift, memory_opt, ratio, offset>::get1(K key) {

        if (offset)
            return offset_pagemap.get1(key);
        else {
            unsigned long long k = (key >> (shift));
            if (sync)
                return sync_pagemap.get(k);
            else
                return unsync_pagemap.get(k);
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
     short &map<K, V, sync, shift, memory_opt, ratio, offset>::get2(K key) {

        if (offset)
            return offset_pagemap.get2(key);
        else {
            throw "Not supported";
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
     int &map<K, V, sync, shift, memory_opt, ratio, offset>::get4(K key) {

        if (offset)
            return offset_pagemap.get4(key);
        else {
            throw "Not supported";
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
     long long &map<K, V, sync, shift, memory_opt, ratio, offset>::get8(K key) {
        if (offset)
            return offset_pagemap.get8(key);
        else {
            throw "Not supported";
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    V &map<K, V, sync, shift, memory_opt, ratio, offset>::getOrDefault(K key, V def) {

        if (offset)
            return offset_pagemap.get(key);
        else {
            unsigned long long k = (key >> (shift));
            if (sync) {
                std::pair<K, V *> p = sync_pagemap.test(k);
                if (p.first)
                    return *p.second;
                else
                    return def;
            } else {
                std::pair<K, V *> p = unsync_pagemap.test(k);
                if (p.first)
                    return *p.second;
                else
                    return def;
            }
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    V &map<K, V, sync, shift, memory_opt, ratio, offset>::operator[](K k) {
        return this->get(k);
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    void map<K, V, sync, shift, memory_opt, ratio, offset>::add_range(K key, V v, size_t size) {

        if (offset) {
            offset_pagemap.add_range(key, v, size);
        } else {
            unsigned long long k = (key >> (shift));
            uint step = (1 << shift);
            uint total = (size >> shift) + (size % step == 0 ? 0 : 1);
            for (int i = 0; i < total; i++) {
                this->get((K) (k + i * step)) = v;
            }
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    void map<K, V, sync, shift, memory_opt, ratio, offset>::remove_range(K key, size_t size) {

        V v;
        if (offset)
            offset_pagemap.add_range(key, v, size);
        else {
            unsigned long long k = (key >> (shift));
            uint step = (1 << shift);
            uint total = (size >> shift) + (size % step == 0 ? 0 : 1);
            for (int i = 0; i < total; i++) {

                if (sync) {
                    std::pair<K, V *> p = sync_pagemap.test(k);
                    if (p.first)
                        *p.second = v;
                } else {
                    std::pair<K, V *> p = unsync_pagemap.test(k);
                    if (p.first)
                        *p.second = v;
                }
            }
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    void map<K, V, sync, shift, memory_opt, ratio, offset>::cpy_range(K k1, K k2, size_t size) {

        if (offset) {
            offset_pagemap.cpy_range(k1, k2, size);
        } else {
            unsigned long long kk1 = (k1 >> (shift));
            unsigned long long kk2 = (k2 >> (shift));
            uint step = (1 << shift);
            uint total = (size >> shift) + (size % step == 0 ? 0 : 1);
            for (int i = 0; i < total; i++) {
                if (sync) {
                    std::pair<K, V *> p = sync_pagemap.test(kk1);
                    if (p.first)
                        sync_pagemap.get(kk2) = *p.second;
                } else {
                    std::pair<K, V *> p = unsync_pagemap.test(kk1);
                    if (p.first)
                        unsync_pagemap.get(kk2) = *p.second;
                }
            }
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    void map<K, V, sync, shift, memory_opt, ratio, offset>::move_range(K k1, K k2, size_t size) {

        if (offset) {
            offset_pagemap.move_range(k1, k2, size);
        } else {
            unsigned long long kk1 = (k1 >> (shift));
            unsigned long long kk2 = (k2 >> (shift));
            uint step = (1 << shift);
            uint total = (size >> shift) + (size % step == 0 ? 0 : 1);
            for (int i = total - 1; i >= 0; i--) {
                if (sync) {
                    std::pair<K, V *> p = sync_pagemap.test(kk1);
                    if (p.first)
                        sync_pagemap.get(kk2) = *p.second;
                } else {
                    std::pair<K, V *> p = unsync_pagemap.test(kk1);
                    if (p.first)
                        unsync_pagemap.get(kk2) = *p.second;
                }
            }
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    V map<K, V, sync, shift, memory_opt, ratio, offset>::or_range(K key, size_t size) {

        if (offset) {
            return offset_pagemap.or_range(key, size);
        } else {
            unsigned long long k = (key >> (shift));
            uint step = (1 << shift);
            uint total = (size >> shift) + (size % step == 0 ? 0 : 1);
            V v;
            for (int i = 0; i < total; i++) {
                if (sync) {
                    std::pair<K, V *> p = sync_pagemap.test(k);
                    if (p.first)
                        v |= *p.second;
                } else {
                    std::pair<K, V *> p = unsync_pagemap.test(k);
                    if (p.first)
                        v |= *p.second;
                }
            }
            return v;
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    void map<K, V, sync, shift, memory_opt, ratio, offset>::remove_range(void *st, size_t size, uint off, uint unit_size) {
        size_t shif = 1<<shift;
        for (uint i = 0; i < (size + shif - 1) / shif; i++) {
            if (this->test((u_int64_t) st + i * shif))
            {
                memset((char *) (&this->get((u_int64_t) st + i * shif)) + off, 0, unit_size);
            }
        }
    }

    template<class K, class V, bool sync, uint shift, uint memory_opt, uint ratio, bool offset>
    void map<K, V, sync, shift, memory_opt, ratio, offset>::add_range(void *st, ull v, size_t size, uint off, uint unit_size) {
        size_t shif = 1<<shift;
        for (uint i = 0; i < (size + shif - 1) / shif; i++) {
            memset((char *) (&this->get((u_int64_t) st + i * shif)) + off, v, unit_size);
        }
    }


    template<class E>
    set<E>::set() {
#ifdef UNIVERSE_SET_SPARSEBITVECTOR_IMPL
        this->universe_sparseset.set_universe(true);
#endif
    }

    template<class E>
    set<E>::set(E e) {
#ifdef UNIVERSE_SET_SPARSEBITVECTOR_IMPL
        this->universe_sparseset.set_universe(true);
        this->universe_sparseset.set(e);
#endif
    }

    template<class E>
    set<E>::set(const set<E> &e) {
        this->universe_id = e.universe_id;
        switch (universe_id) {
#ifdef UNIVERSE_SET_SPARSEBITVECTOR_IMPL
            case 0:
                this->universe_sparseset = e.universe_sparseset;
                break;
#endif
            default:
                throw "No matched impl";
        }
    }

    template<class E>
    bool set<E>::find(E e) {
#ifdef UNIVERSE_SET_SPARSEBITVECTOR_IMPL
        return this->universe_sparseset.test(e);
#endif
    }

    template<class E>
    void set<E>::add(E e) {
#ifdef UNIVERSE_SET_SPARSEBITVECTOR_IMPL
        this->universe_sparseset.set(e);
#endif
    }

    template<class E>
    void set<E>::remove(E e) {
#ifdef UNIVERSE_SET_SPARSEBITVECTOR_IMPL
        this->universe_sparseset.reset(e);
#endif
    }

    template<class E>
    bool set<E>::empty() {
#ifdef UNIVERSE_SET_SPARSEBITVECTOR_IMPL
        return this->universe_sparseset.empty();
#endif
    }

    template<class E>
    set<E> set<E>::operator+(bottom::set<E> &another) {
        set<E> ans = *this;
        ans.universe_sparseset += another.bottom_set;
        return ans;
    }

    template<class E>
    set<E> set<E>::operator-(bottom::set<E> &another) {
        set<E> ans = *this;
        ans.universe_sparseset -= another.bottom_set;
        return ans;
    }

    template<class E>
    bottom::set<E> set<E>::operator&(bottom::set<E> &another) {
        bottom::set<E> ans = another;
        ans.bottom_set -= this->universe_sparseset;
        return ans;
    }

    template<class E>
    set<E> set<E>::operator|(bottom::set<E> &another) {
        set<E> ans = *this;
        ans.universe_sparseset |= another.bottom_set;
        return ans;
    }

    template<class E>
    set<E> &set<E>::operator+=(bottom::set<E> &another) {
        this->universe_sparseset += another.bottom_set;
        return *this;
    }

    template<class E>
    set<E> &set<E>::operator-=(bottom::set<E> &another) {
        this->universe_sparseset -= another.bottom_set;
        return *this;
    }

    template<class E>
    set<E> &set<E>::operator|=(bottom::set<E> &another) {
        this->universe_sparseset |= another.bottom_set;
        return *this;
    }

    template<class E>
    set<E> set<E>::operator+(set<E> &another) {
        set<E> ans = *this;
        ans.universe_sparseset += another.universe_sparseset;
        return ans;
    }

    template<class E>
    set<E> set<E>::operator&(set<E> &another) {
        set<E> ans = *this;
        ans.universe_sparseset &= another.universe_sparseset;
        return ans;
    }

    template<class E>
    set<E> set<E>::operator|(set<E> &another) {
        set<E> ans = *this;
        ans.universe_sparseset |= another.universe_sparseset;
        return ans;
    }

    template<class E>
    set<E> &set<E>::operator+=(set<E> &another) {
        this->universe_sparseset += another.universe_sparseset;
        return *this;
    }

    template<class E>
    set<E> &set<E>::operator|=(set<E> &another) {
        this->universe_sparseset |= another.universe_sparseset;
        return *this;
    }

    template<class E>
    set<E> &set<E>::operator&=(set<E> &another) {
        this->universe_sparseset &= another.universe_sparseset;
        return *this;
    }


}


namespace bottom {
    template<class K, class V, bool sync>
    map<K, V, sync>::map() {
        if (sync)
        pthread_spin_init(&l, 0);
    }

    template<class K, class V, bool sync>
    bool map<K, V, sync>::find(K k) {
        if(sync)
        pthread_spin_lock(&l);
#ifdef BOTTOM_MAP_STLMAP_IMPL
        bool ans = this->bottom_stlmap.find(k) != this->bottom_stlmap.end();
        if(sync)
        pthread_spin_unlock(&l);
        return ans;
#endif

    }

    template<class K, class V, bool sync>
    unsigned long map<K, V, sync>::size() {

#ifdef BOTTOM_MAP_STLMAP_IMPL
        return this->bottom_stlmap.size();
#endif
    }

    template<class K, class V, bool sync>
    V &map<K, V, sync>::get(K k) {
        if(sync)
        pthread_spin_lock(&l);
#ifdef BOTTOM_MAP_STLMAP_IMPL
        auto &ans = this->bottom_stlmap[k];
        if(sync)
        pthread_spin_unlock(&l);
        return ans;
#endif
    }

    template<class K, class V, bool sync>
    V &map<K, V, sync>::operator[](K k) {
#ifdef BOTTOM_MAP_STLMAP_IMPL
        return this->get(k);
#endif
    }

    template<class K, class V, bool sync>
    void map<K, V, sync>::remove(K k) {
        if(sync)
        pthread_spin_lock(&l);
#ifdef BOTTOM_MAP_STLMAP_IMPL
        this->bottom_stlmap.erase(k);
#endif
        if(sync)
        pthread_spin_unlock(&l);
    }

    template<class K, class V, bool sync>
    void map<K, V, sync>::add_range(K k, V v, int size) {
#ifdef BOTTOM_MAP_STLMAP_IMPL
        if(sync)
        pthread_spin_lock(&l);
        for (int i = 0; i < size; i++) {

            this->bottom_stlmap[(K) ((int *) k + i)] = v;

        }
        if(sync)
        pthread_spin_unlock(&l);
#endif
    }

    template<class K, class V, bool sync>
    void map<K, V, sync>::remove_range(K k, int size) {
#ifdef BOTTOM_MAP_STLMAP_IMPL
        if(sync)
        pthread_spin_lock(&l);
        for (int i = 0; i < size; i++) {
            this->remove(k + i);
        }
        if(sync)
        pthread_spin_unlock(&l);
#endif
    }


    template<class E>
    set<E>::set() {

    }

    template<class E>
    set<E>::set(E e) {
#ifdef BOTTOM_SET_STLSET_IMPL
        this->bottom_set.insert(e);
#endif
#ifdef BOTTOM_SET_SPARSESET_IMPL
        this->bottom_set.set(e);
#endif
    }

    template<class E>
    set<E>::set(const set<E> &e) {
        this->bottom_id = e.bottom_id;
        switch (bottom_id) {
#ifdef BOTTOM_SET_STLSET_IMPL
            case 0:
                this->bottom_set = e.bottom_set;
                break;
#endif
#ifdef BOTTOM_SET_SPARSESET_IMPL
            case 1:
                this->bottom_set = e.bottom_set;
                break;
#endif
            default:
                throw "No matched impl";
        }
    }

    template<class E>
    bool set<E>::find(E e) {
#ifdef BOTTOM_SET_STLSET_IMPL
        return this->bottom_set.find(e) != this->bottom_set.end();
#endif
#ifdef BOTTOM_SET_SPARSESET_IMPL
        return this->bottom_set.test(e);
#endif
    }

    template<class E>
    void set<E>::add(E e) {
#ifdef BOTTOM_SET_STLSET_IMPL
        this->bottom_set.insert(e);
#endif
#ifdef BOTTOM_SET_SPARSESET_IMPL
        this->bottom_set.set(e);
#endif
    }

    template<class E>
    void set<E>::remove(E e) {
#ifdef BOTTOM_SET_STLSET_IMPL
        this->bottom_set.erase(e);
#endif
#ifdef BOTTOM_SET_SPARSESET_IMPL
        this->bottom_set.reset(e);
#endif
    }

    template<class E>
    unsigned long set<E>::size() {
#ifdef BOTTOM_SET_STLSET_IMPL
        return this->bottom_set.size();
#endif
#ifdef BOTTOM_SET_SPARSESET_IMPL
        return this->bottom_set.count();
#endif
    }

    template<class E>
    auto set<E>::begin() {
#ifdef BOTTOM_SET_STLSET_IMPL
        return;this->bottom_set.begin();
#endif
#ifdef BOTTOM_SET_SPARSESET_IMPL
        this->bottom_set.begin();
#endif
    }

    template<class E>
    auto set<E>::end() {
#ifdef BOTTOM_SET_STLSET_IMPL
        return;this->bottom_set.end();
#endif
#ifdef BOTTOM_SET_SPARSESET_IMPL
        this->bottom_set.end();
#endif
    }

    template<class E>
    set<E> set<E>::operator+(set<E> &another) {
        set<E> ans = *this;
        ans.bottom_set += another.bottom_set;
        return ans;
    }

    template<class E>
    set<E> set<E>::operator-(set<E> &another) {
        set<E> ans = *this;
        ans.bottom_set -= another.bottom_set;
        return ans;
    }

    template<class E>
    set<E> set<E>::operator&(set<E> &another) {
        set<E> ans = *this;
        ans.bottom_set &= another.bottom_set;
        return ans;
    }

    template<class E>
    set<E> set<E>::operator|(set<E> &another) {
        set<E> ans = *this;
        ans.bottom_set |= another.bottom_set;
        return ans;
    }

    template<class E>
    set<E> &set<E>::operator+=(set<E> &another) {
        this->bottom_set += another.bottom_set;
        return *this;
    }

    template<class E>
    set<E> &set<E>::operator-=(set<E> &another) {
        this->bottom_set -= another.bottom_set;
        return *this;
    }


    template<class E>
    set<E> &set<E>::operator|=(set<E> &another) {
        this->bottom_set |= another.bottom_set;
        return *this;
    }

    template<class E>
    set<E> set<E>::operator&=(set<E> &another) {
        this->bottom_set &= another.bottom_set;
        return *this;
    }

    template<class E>
    universe::set<E> set<E>::operator+(universe::set<E> &another) {
        return another + *this;
    }

    template<class E>
    set<E> set<E>::operator-(universe::set<E> &another) {
        set<E> ans = *this;
        ans.bottom_set -= another.universe_sparseset;
        return ans;
    }

    template<class E>
    set<E> set<E>::operator&(universe::set<E> &another) {
        set<E> ans = *this;
        ans.bottom_set &= another.universe_sparseset;
        return ans;
    }

    template<class E>
    universe::set<E> set<E>::operator|(universe::set<E> &another) {
        return another | *this;
    }

    template<class E>
    set<E> &set<E>::operator-=(universe::set<E> &another) {
        this->bottom_set -= another.universe_sparseset;
        return *this;
    }


    template<class E>
    set<E> &set<E>::operator&=(universe::set<E> &another) {
        this->bottom_set &= another.universe_sparseset;
        return *this;
    }
}

namespace rt_lib {
    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    map<K, V, uni, sync, shift, memory_opt>::map() {
    }


    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    bool map<K, V, uni, sync, shift, memory_opt>::find(K k) {
        if (uni) {
            return universe_map.find(k);
        } else {
            return bottom_map.find(k);
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    unsigned long map<K, V, uni, sync, shift, memory_opt>::size() {
        if (uni) {
            return universe_map.size();
        } else {
            return bottom_map.size();
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    V &map<K, V, uni, sync, shift, memory_opt>::get(K k) {
        if (uni) {
            return universe_map.get(k);
        } else {
            return bottom_map.get(k);
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    ull map<K, V, uni, sync, shift, memory_opt>::get(K k, size_t size) {
        if (uni) {
            return universe_map.get(k, size);
        } else {
            throw "Not supported";
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    void map<K, V, uni, sync, shift, memory_opt>::set(K k,  ull v, size_t size) {
        if (uni) {
            universe_map.set(k,v,size);
        } else {
            throw "Not supported";
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    unsigned char &map<K, V, uni, sync, shift, memory_opt>::get1(K k) {
        if (uni) {
            return universe_map.get1(k);
        } else {
            throw "Bottom map not supported";
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    unsigned short &map<K, V, uni, sync, shift, memory_opt>::get2(K k) {
        if (uni) {
            return universe_map.get2(k);
        } else {
            throw "Bottom map not supported";
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    unsigned int &map<K, V, uni, sync, shift, memory_opt>::get4(K k) {
        if (uni) {
            return universe_map.get4(k);
        } else {
            throw "Bottom map not supported";
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    unsigned long long &map<K, V, uni, sync, shift, memory_opt>::get8(K k) {
        if (uni) {
            return universe_map.get8(k);
        } else {
            throw "Bottom map not supported";
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    V &map<K, V, uni, sync, shift, memory_opt>::operator[](K k) {
        return this->get(k);
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    void map<K, V, uni, sync, shift, memory_opt>::remove(K k) {
        if (uni) {
            throw "Universe map doesn't support remove";
        } else {
            return bottom_map.remove(k);
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    void map<K, V, uni, sync, shift, memory_opt>::remove_range(K k, size_t size) {
        if (uni) {
            universe_map.remove_range(k, size);
        } else {
            return bottom_map.remove_range(k, size);
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    void map<K, V, uni, sync, shift, memory_opt>::add_range(K k, V v, size_t size) {
        if (uni) {
            this->universe_map.add_range(k, v, size);
        } else {
            return bottom_map.add_range(k, v, size);
        }
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    bool map<K, V, uni, sync, shift, memory_opt>::check_range(K k, size_t size, V v) {
        for (int i = 0; i < size; i++) {
            K kk = (K) ((int *) k + i);
            if (this->get(kk) != v)
                return false;
        }
        return true;
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    V &map<K, V, uni, sync, shift, memory_opt>::getOrDefault(K k, V def) {
        if (uni)
            return universe_map.getOrDefault(k, def);
        else
            return def;
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    void map<K, V, uni, sync, shift, memory_opt>::cpy_range(K k1, K k2, size_t size) {
        if (uni) {
            universe_map.cpy_range(k1, k2, size);
        } else
            throw "Bottom Map doesn't  support";
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    void map<K, V, uni, sync, shift, memory_opt>::move_range(K k1, K k2, size_t size) {
        if (uni) {
            universe_map.move_range(k1, k2, size);
        } else
            throw "Bottom Map doesn't  support";
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    V map<K, V, uni, sync, shift, memory_opt>::or_range(K k, size_t size) {
        if (uni) {
            return universe_map.or_range(k, size);
        } else
            throw "Bottom Map doesn't  support";
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    void map<K, V, uni, sync, shift, memory_opt>::remove_range(void *k, size_t size, uint offset, uint unit_size) {
        if (uni) {
            universe_map.remove_range(k, size, offset, unit_size);
        } else
            throw "Bottom map not support";
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    void map<K, V, uni, sync, shift, memory_opt>::add_range(void *k, ull v, size_t size, uint offset, uint unit_size) {
        if (uni) {
            universe_map.add_range(k, v, size, offset, unit_size);
        } else
            throw "Bottom map not support";
    }

    template<class K, class V, bool uni, bool sync, int shift, int memory_opt>
    bool map<K, V, uni, sync, shift, memory_opt>::check_range(void *k, size_t size, u_char v, uint off, uint unit_size) {
        size_t shif = 1<<shift;
        for (int i = 0; i < (size + shif - 1) / shif; i++) {
            u_char *p = (u_char *) (&this->get((void*)((u_int64_t) k + i * shif)));
            p = &p[off];
            for (uint j = 0; j < unit_size; j++) {
                if (p[j] != v)
                    return false;
            }
        }
        return true;
    }

    template<class E>
    set<E>::set() {
        universe = false;
    }

    template<class E>
    set<E>::set(bool uni) {
        universe = uni;
    }

    template<class E>
    bool set<E>::find(E e) {
        if (universe) {
            return universe_set.find(e);
        } else
            return bottom_set.find(e);
    }

    template<class E>
    void set<E>::add(E e) {
        if (universe) {
            universe_set.add(e);
        } else
            bottom_set.add(e);
    }

    template<class E>
    void set<E>::remove(E e) {
        if (universe) {
            universe_set.remove(e);
        } else
            bottom_set.remove(e);
    }

    template<class E>
    bool set<E>::empty() {
        if (universe) {
            return universe_set.empty();
        } else
            return bottom_set.size() == 0;
    }

    template<class E>
    set<E> set<E>::operator+(rt_lib::set<E> &another) {
        if (universe) {
            if (another.universe) {
                set<E> ans = *this;
                ans.universe_set += another.universe_set;
                return ans;
            } else {
                set<E> ans = *this;
                ans.universe_set += another.bottom_set;
                return ans;
            }
        } else {
            if (another.universe) {
                //bottom + universe = universe
                set<E> ans = another;
                ans.universe_set += this->bottom_set;
                return ans;
            } else {
                set<E> ans = *this;
                ans.bottom_set += another.bottom_set;
                return ans;
            }
        }
    }

    template<class E>
    set<E> set<E>::operator-(set<E> &another) {
        if (universe) {
            if (another.universe) {
                throw "Universe Set - not supported";
            } else {
                set<E> ans = *this;
                ans.universe_set -= another.bottom_set;
                return ans;
            }
        } else {
            if (another.universe) {
                set<E> ans = *this;
                ans.bottom_set -= another.universe_set;
                return ans;
            } else {
                set<E> ans = *this;
                ans.bottom_set -= another.bottom_set;
                return ans;
            }
        }
    }

    template<class E>
    set<E> set<E>::operator&(set<E> &another) {
        if (universe) {
            if (another.universe) {
                set<E> ans = *this;
                ans.universe_set &= another.universe_set;
                return ans;
            } else {
                set<E> ans = another;
                ans.bottom_set &= this->universe_set;
                return ans;
            }
        } else {
            if (another.universe) {
                set<E> ans = *this;
                ans.bottom_set &= another.universe_set;
                return ans;
            } else {
                set<E> ans = *this;
                ans.bottom_set &= another.bottom_set;
                return ans;
            }
        }
    }

    template<class E>
    set<E> set<E>::operator|(set<E> &another) {
        return *this & another;
    }

    template<class E>
    set<E> &set<E>::operator+=(set<E> &another) {
        if (universe) {
            if (another.universe) {
                this->universe_set += another.universe_set;
                return *this;
            } else {
                this->universe_set += another.bottom_set;
                return *this;
            }
        } else {
            if (another.universe) {
                //bottom + universe = universe
                this->universe_set = another.universe_set;
                this->universe_set += this->bottom_set;
                this->universe = true;
                return *this;
            } else {
                this->bottom_set += another.bottom_set;
                return *this;
            }
        }
    }

    template<class E>
    set<E> &set<E>::operator-=(set<E> &another) {
        if (universe) {
            if (another.universe) {
                throw "Universe Set - not supported";
            } else {
                this->universe_set -= another.bottom_set;
                return *this;
            }
        } else {
            if (another.universe) {
                this->bottom_set -= another.universe_set;
                return *this;
            } else {
                this->bottom_set -= another.bottom_set;
                return *this;
            }
        }
    }

    template<class E>
    set<E> &set<E>::operator&=(set<E> &another) {
        if (universe) {
            if (another.universe) {
                this->universe_set &= another.universe_set;
                return *this;
            } else {
                this->bottom_set = another.bottom_set;
//                this->bottom_set &= this->universe_set;
//                assert(this->bottom_set.size() > 0);
                this->universe = false;
                return *this;
            }
        } else {
            if (another.universe) {
                this->bottom_set &= another.universe_set;
                return *this;
            } else {
                this->bottom_set &= another.bottom_set;
                return *this;
            }
        }
    }

    template<class E>
    set<E> &set<E>::operator|=(set<E> &another) {
        return *this += another;
    }

    template<class E>
    void set<E>::remove_range(E e, int size) {
        for (int i = 0; i < size; i++) {
            this->remove(e + i);
        }
    }

    template<class E>
    void set<E>::add_range(E e, int size) {
        for (int i = 0; i < size; i++) {
            this->add(e + i);
        }
    }

    template<class E>
    size_t set<E>::size() {
        if (universe) {
            return 4;
        } else {
            return bottom_set.size();
        }
    }


}