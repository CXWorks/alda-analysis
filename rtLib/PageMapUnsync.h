/*
 * Copyright (C) 2016 David Devecsery
 */

#ifndef UTIL_UNSYNC_DATATYPES_PAGEMAP_H_
#define UTIL_UNSYNC_DATATYPES_PAGEMAP_H_

#include <cassert>
#include <iterator>
#include <array>
#include <limits>
#include <memory>
#include <atomic>
#include <pthread.h>

namespace unsync {


    template<size_t bit, size_t... bits>
    struct PageSizes {
        static const size_t size = bit;

        typedef PageSizes<bits...> next;
    };

    template<size_t bit>
    struct PageSizes<bit> {
        static const size_t size = bit;
    };

    template<typename bit_decl = PageSizes<12, 20, 20, 12>,
            typename Key = uintptr_t, typename T = size_t,
            typename base_alloc = std::allocator<T>>
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

        typedef base_alloc alloc;

        // An internal page class
        template<typename sizes>
        class ShadowPageInternal {
            //{{{
        public:
            typedef ShadowPageInternal<typename sizes::next> NextPageType;

            static const size_t bits = sizes::size;
            static const size_t PageSize = (1 << bits);
            static const size_t Mask = PageSize - 1;
            static const size_t EntriesPerPage = NextPageType::EntriesPerPage * PageSize;
            static const size_t ShiftSize = NextPageType::ShiftSize + bits;

            ShadowPageInternal() {
                for (size_t i = 0; i < PageSize; i++) {
                    map_[i] = nullptr;
                }
            }

            const std::pair<bool, pointer> test(const_key_reference key) {
                auto mask_key = keyMask(key);

                if (map_[mask_key] == nullptr) {
                    return std::make_pair(false, nullptr);
                }

                return map_[mask_key]->test(key);
            }

            const std::pair<bool, const_pointer> test(const_key_reference key) const {
                auto mask_key = keyMask(key);

                if (map_[mask_key] == nullptr) {
                    return std::make_pair(false, nullptr);
                }

                return map_[mask_key]->test(key);
            }


            reference get(const_key_reference key, alloc &a) {
                auto mask_key = keyMask(key);

                if (map_[mask_key] == nullptr) {
                    auto next_page = allocate_page(a);
                    map_[mask_key] = next_page;

                }

                return map_[mask_key]->get(key, a);
            }

        private:
            NextPageType *allocate_page(alloc &a) {
                // Rebind to the correct allocator, and alloc
                typedef typename std::allocator_traits<base_alloc>::template rebind_alloc<NextPageType>
                        page_alloc;
                auto rebind = page_alloc(a);
                auto ret = rebind.allocate(1, nullptr);
                try {
                    std::allocator_traits<page_alloc>::construct(rebind, ret);
                } catch (...) {
                    rebind.deallocate(ret, 1);
                    throw;
                }

                return ret;
            }

            static uintptr_t keyMask(key_type key) {
                return (key >> NextPageType::ShiftSize) & Mask;
            }

//            struct Deleter {
//                Deleter() = default;
//                Deleter(alloc a) : a_(a) { }
//
//                void operator()(NextPageType *del) {
//                    // Rebind to the correct allocator, and alloc
//                    typedef typename std::allocator_traits<base_alloc>::template rebind_alloc<NextPageType>
//                            page_alloc;
//                    auto rebind = page_alloc(a_);
//                    std::allocator_traits<page_alloc>::destroy(rebind, del);
//                    rebind.deallocate(del, 1);
//                }
//
//                alloc a_;
//            };
            std::array<NextPageType *, PageSize> map_;
            //}}}
        };

        // A specialization for our bottom-level page
        template<size_t page_bits>
        class ShadowPageInternal<PageSizes<page_bits>> {
            //{{{
        public:
            static const size_t bits = page_bits;
            static const size_t PageSize = (1 << bits);
            static const size_t Mask = PageSize - 1;
            static const size_t EntriesPerPage = PageSize;
            static const size_t ShiftSize = bits;

            reference get(const_key_reference key, alloc &) {
                auto mask_key = key & Mask;

                return map_[mask_key];
            }

            const std::pair<bool, pointer> test(const_key_reference key) {
                auto mask_key = key & Mask;
                return std::make_pair(true, &map_[mask_key]);
            }

            const std::pair<bool, const_pointer> test(const_key_reference key) const {
                auto mask_key = key & Mask;

                return std::make_pair(true, &map_[mask_key]);
            }


        private:
            std::array<value_type, PageSize> map_;
            //}}}
        };

        std::pair<bool, pointer> test(const_key_reference key) {
            return map_.test(key);
        }

        std::pair<bool, const_pointer> &test(const_key_reference key) const {
            return map_.test(key);
        }

        reference get(const_key_reference key) {
            return map_.get(key, alloc_);
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


    private:
        ShadowPageInternal<bit_decl> map_;
        unsigned long size = 0;
        alloc alloc_;
        //}}}
    };


}  // namespace util

#endif  // UTIL_DATATYPES_PAGEMAP_H_
