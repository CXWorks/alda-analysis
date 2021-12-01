//
// Created by cxworks on 19-11-24.
//

#ifndef AUTO_ANALYSIS_BIT_VECTOR_H
#define AUTO_ANALYSIS_BIT_VECTOR_H

#include <stdlib.h>
#include <climits>

namespace rt_lib {
    template<size_t T, size_t unit_len = 8, int len = T / unit_len + (T % unit_len == 0 ? 0 : 1)>
    class bit_vector {
    private:
        unsigned char bitsets[len];
    public:
        bit_vector() {}


        bit_vector(bool universe) {
            if (universe) {
                for (int i = 0; i < len; ++i) {
                    bitsets[i] = UCHAR_MAX;
                }
            }
        }

        bool empty() {
            for (int i = 0; i < len; i++) {
                if (bitsets[i] != 0)
                    return false;
            }
            return true;
        }

        bool find(int id) {
            id -= 1;
            int idx = id / unit_len;
            int rem = id % unit_len;
            auto mask = static_cast<unsigned char>(1 << rem);
            return (bitsets[idx] & mask) != 0;
        }

        size_t size() {
            size_t ans = 0;
            for (int i = 0; i < len; ++i) {
                for (int j = 0; j < unit_len; ++j) {
                    auto mask = static_cast<unsigned char>(1 << j);
                    if ((bitsets[i] & mask) != 0)
                        ans++;
                }
            }
            return ans;
        }

        void add(int id) {
            id -= 1;
            int idx = id / unit_len;
            int rem = id % unit_len;
            auto mask = static_cast<unsigned char>(1 << rem);
            bitsets[idx] |= mask;
        }

        void remove(int id) {
            id -= 1;
            int idx = id / unit_len;
            int rem = id % unit_len;
            auto mask = static_cast<unsigned char>(1 << rem);
            mask ^= UCHAR_MAX;
            bitsets[idx] &= mask;
        }

        bit_vector<T> &operator&=(bit_vector<T> &another) {
            for (int i = 0; i < len; ++i) {
                bitsets[i] &= another.bitsets[i];
            }
            return *this;
        }

        bit_vector<T> operator&(bit_vector<T> &another) {
            bit_vector<T> ans;
            for (int i = 0; i < len; ++i) {
                ans.bitsets[i] = bitsets[i] & another.bitsets[i];
            }
            return ans;
        }
    };


    template<class K, class V, size_t T = 16>
    class array_map {
    private:
        V v[T];
    public:
        array_map() = default;

        ~array_map() {}

        array_map(const array_map<K, V, T> &another) {
            for (int i = 0; i < T; ++i) {
                v[i] = another.v[i];
            }
        }

        V &get(K k) {
            return v[k - 1];
        }

        V &operator[](K k) {
            return get(k);
        }

        void remove(K k) {
            V nv;
            v[k - 1] = nv;
        }

        void remove_range(K k, int size) {
            V nv;
            for (int i = 0; i < size; i++) {

                v[k + i - 1] = nv;
            }
        }

        bool operator<(array_map<K, V, T> &another) {
            for (int i = 0; i < T; i++) {
                if (v[i] >= another.v[i])
                    return false;
            }
            return true;
        }

        bool operator>(array_map<K, V, T> &another) {
            for (int i = 0; i < T; i++) {
                if (v[i] <= another.v[i])
                    return false;
            }
            return true;
        }

        bool operator>=(array_map<K, V, T> &another) {
            for (int i = 0; i < T; i++) {
                if (v[i] < another.v[i])
                    return false;
            }
            return true;
        }

        bool operator<=(array_map<K, V, T> &another) {
            for (int i = 0; i < T; i++) {
                if (v[i] > another.v[i])
                    return false;
            }
            return true;
        }

        bool operator==(array_map<K, V, T> &another) {

            for (int i = 0; i < T; i++) {
                if (v[i] != another.v[i])
                    return false;
            }
            return true;
        }

        array_map &operator|=(array_map<K, V, T> &another) {

            for (int i = 0; i < T; i++) {
                v[i] = v[i] > another.v[i] ? v[i] : another.v[i];
            }
            return *this;
        }

    };

    template<class K, class V, size_t T = 16>
    class small_hash_map {
    private:
        K k[T];
        V v[T];
    public:
        small_hash_map() = default;

        ~small_hash_map() {}

        small_hash_map(const small_hash_map<K, V, T> &another) {
            for (int i = 0; i < T; ++i) {
                k[i] = another.k[i];
                v[i] = another.v[i];
            }
        }

        int find_loc(K ks) {
            int i = 0;
            while (i < 16) {
                if (k[(ks & (T - 1) + i) % 16] == ks)
                    return (ks & (T - 1) + i) % 16;
                else if (k[(ks & (T - 1) + i) % 16] == 0)
                    return (ks & (T - 1) + i) % 16;
                i++;
            }
            throw "Not Found";
        }

        int find_loc_safe(K ks) {
            int i = 0;
            while (i < 16) {
                if (k[(ks & (T - 1) + i) % 16] == ks)
                    return (ks & (T - 1) + i) % 16;
                else if (k[(ks & (T - 1) + i) % 16] == 0)
                    return -1;
                i++;
            }
            return -1;
        }

        V &get(K ks) {
            int i = find_loc(ks);
            return v[i];
        }

        V &operator[](K ks) {
            return get(ks);
        }

        void remove(K ks) {
            V nv;
            K nk;
            int i = find_loc_safe(ks);
            if (i != -1) {
                k[i] = nk;
                v[i] = nv;
            }

        }

        void remove_range(K ks, int size) {
            for (int i = 0; i < size; i++) {
                remove(k + i - 1);
            }
        }

        bool operator<(small_hash_map<K, V, T> &another) {
            for (int i = 0; i < T; i++) {
                if (k[i] != 0 && v[i] >= another.v[i])
                    return false;
            }
            return true;
        }

        bool operator>(small_hash_map<K, V, T> &another) {
            for (int i = 0; i < T; i++) {
                if (k[i] != 0 && v[i] <= another.v[i])
                    return false;
            }
            return true;
        }

        bool operator>=(small_hash_map<K, V, T> &another) {
            for (int i = 0; i < T; i++) {
                if (k[i] != 0 && v[i] < another.v[i])
                    return false;
            }
            return true;
        }

        bool operator<=(small_hash_map<K, V, T> &another) {
            for (int i = 0; i < T; i++) {
                if (k[i] != 0 && v[i] > another.v[i])
                    return false;
            }
            return true;
        }

        bool operator==(small_hash_map<K, V, T> &another) {

            for (int i = 0; i < T; i++) {
                if (k[i] != 0 && v[i] != another.v[i])
                    return false;
            }
            return true;
        }

        small_hash_map &operator|=(small_hash_map<K, V, T> &another) {

            for (int i = 0; i < T; i++) {
                k[i] |= another.k[i];
                v[i] |= v[i] > another.v[i];
            }
            return *this;
        }

    };
}
#endif //AUTO_ANALYSIS_BIT_VECTOR_H
