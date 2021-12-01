/*
 * Copyright (C) 2016 David Devecsery
 */
#ifndef UTIL_BITMAP_SPARSE_BITMAP_H_
#define UTIL_BITMAP_SPARSE_BITMAP_H_

#include <array>
#include <bitset>
#include <iterator>
#include <list>
#include <ostream>
#include <cassert>

namespace util {

    template<size_t bits_per_field, class idxt>
    class BitmapNode {
        //{{{
    public:
        // Note: we use c-type here as gcc makes it a word size
        // Constant Internals {{{
        static const int32_t BitsPerSet = sizeof(int); //  NOLINT
        static const size_t NumBitsets = bits_per_field / BitsPerSet;
        //}}}

        // Typedefs {{{
        typedef typename std::array<std::bitset<BitsPerSet>, NumBitsets> bmap;
        //}}}

        // Static Asserts {{{
        static_assert(bits_per_field % BitsPerSet == 0,
                      "bits_per_field must be divsible by BitsPerSet");
        //}}}

        // Constructors {{{
        explicit BitmapNode(size_t idx, bool universe) : index_(idx) {
            if (universe)
                for (auto &bs : bitmap_) {
                    bs.set();
                }
        }
        // }}}

        // Operators {{{
        // Equality {{{
        bool operator==(const BitmapNode &rhs) const {
            return index() == rhs.index() && bitmap_ == rhs.bitmap_;
        }

        bool operator!=(const BitmapNode &rhs) const {
            return !(*this == rhs);
        }
        //}}}

        // Assignment/manipulation {{{

        BitmapNode &operator+=(const BitmapNode &rhs) {
            assert(index() == rhs.index());

            return *this |= rhs;
        }

        BitmapNode &operator|=(const BitmapNode &rhs) {
            assert(index() == rhs.index());

            bool ch = false;
            auto rhs_it = std::begin(rhs.bitmap_);

            for (auto &bs : bitmap_) {
                bs |= *rhs_it;
                ++rhs_it;
            }

            return *this;
        }

        BitmapNode &operator&=(const BitmapNode &rhs) {
            assert(index() == rhs.index());

            auto rhs_it = std::begin(rhs.bitmap_);
            for (auto &bs : bitmap_) {

                bs &= *rhs_it;

                ++rhs_it;
            }

            return *this;
        }

        BitmapNode &operator-=(const BitmapNode &rhs) {
            assert(rhs.index() == index());
            BitmapNode ret = *this;

            auto rhs_it = std::begin(rhs.bitmap_);
            for (auto &bs : ret.bitmap_) {
                bs &= ~(*rhs_it);
                ++rhs_it;
            }

            return *this;
        }
        //}}}

        // Difference {{{
        BitmapNode operator-(const BitmapNode &rhs) {
            assert(rhs.index() == index());
            BitmapNode ret = *this;

            auto rhs_it = std::begin(rhs.bitmap_);
            for (auto &bs : ret.bitmap_) {
                bs &= ~(*rhs_it);

                ++rhs_it;
            }

            return std::move(ret);
        }

        BitmapNode operator+(const BitmapNode &rhs) {
            assert(rhs.index() == index());
            BitmapNode ret = *this;

            auto rhs_it = std::begin(rhs.bitmap_);
            for (auto &bs : ret.bitmap_) {
                bs |= (*rhs_it);

                ++rhs_it;
            }

            return std::move(ret);
        }

        BitmapNode operator&(const BitmapNode &rhs) {
            assert(rhs.index() == index());
            BitmapNode ret = *this;

            auto rhs_it = std::begin(rhs.bitmap_);
            for (auto &bs : ret.bitmap_) {
                bs &= (*rhs_it);

                ++rhs_it;
            }

            return std::move(ret);
        }

        BitmapNode operator|(const BitmapNode &rhs) {
            assert(rhs.index() == index());

            return *this + rhs;
        }
        //}}}
        //}}}

        // Equality/Intersection operations {{{
        bool bitmapEqual(const BitmapNode &rhs) const {
            return bitmap_ == rhs.bitmap_;
        }

        bool orWithIntersect(const BitmapNode &rhs,
                             const BitmapNode &is) {
            assert(index() == rhs.index());
            assert(index() == is.index());

            bool ch = false;
            auto rhs_it = std::begin(rhs.bitmap_);
            auto is_it = std::begin(is.bitmap_);

            for (auto &bs : bitmap_) {
                auto c1 = bs.to_ulong();

                bs |= (*rhs_it & *is_it);

                // FIXME: Should be enabled in llvm code, but testcase doesn't enforce
                // this
                // assert((bs & std::bitset<BitsPerSet>(*is_it).flip()).none());

                ch |= (c1 != bs.to_ulong());
                ++rhs_it;
                ++is_it;
            }

            return ch;
        }

        static BitmapNode Intersect(const BitmapNode &rhs,
                                    const BitmapNode &intersect) {
            BitmapNode ret(rhs);

            rhs &= intersect;

            return rhs;
        }

        bool intersects(const BitmapNode &rhs) const {
            if (index() != rhs.index()) {
                return false;
            }

            auto rhs_it = std::begin(rhs.bitmap_);
            for (auto &elm : bitmap_) {
                if (!(elm & *rhs_it).none()) {
                    return true;
                }
                ++rhs_it;
            }

            return false;
        }
        //}}}

        // Test/set/reset {{{
        bool test(size_t idx) const {
            size_t bitset = idx / BitsPerSet;
            size_t offs = idx % BitsPerSet;
            return bitmap_[bitset].test(offs);
        }

        void set(size_t idx) {
            size_t bitset = idx / BitsPerSet;
            size_t offs = idx % BitsPerSet;
            bitmap_[bitset].set(offs);
        }

        void reset(size_t idx) {
            size_t bitset = idx / BitsPerSet;
            size_t offs = idx % BitsPerSet;
            bitmap_[bitset].reset(offs);
        }
        //}}}

        // Count/none {{{
        size_t count() const {
            size_t ret = 0;
            for (auto &bset : bitmap_) {
                ret += bset.count();
            }
            return ret;
        }

        bool none() const {
            for (auto &bs : bitmap_) {
                if (!bs.none()) {
                    return false;
                }
            }

            return true;
        }
        //}}}

        // Accessors {{{
        size_t index() const {
            return index_;
        }

        unsigned long getUl(size_t idx) const { //  NOLINT
            return bitmap_[idx].to_ulong();
        }
        //}}}

        // Hash {{{
        size_t hash() const {
            static_assert(sizeof(size_t) == sizeof(unsigned long),  // NOLINT
                          "hash assumes sizeof size_t == sizeof ulong");
            size_t ret = index() << 11 ^index();
            for (auto &elm : bitmap_) {
                // Mix ret
                ret ^= (ret << 11);
                ret ^= std::hash<unsigned long>()(elm.to_ulong());  // NOLINT
                // ret ^= std::hash<std::bitset<BitsPerSet>>()(elm);
            }
            return ret;
        }
        //}}}

    private:
        bmap bitmap_;
        idxt index_;
        //}}}
    };

//todo tempory use std alloc to replace
    template<typename id_type = int32_t, size_t bits_per_field = 16,
            typename alloc  = std::allocator<BitmapNode<bits_per_field, id_type>>>
    class SparseBitmap {
        //{{{
    public:
        typedef BitmapNode<bits_per_field, id_type> node;
        typedef typename std::list<node, alloc> bitmap_list;

        // Constructors {{{
        explicit SparseBitmap(const alloc &a = alloc()) : elms_(a) {}

        SparseBitmap(const SparseBitmap &s) :
                elms_(std::begin(s.elms_), std::end(s.elms_))
        /*
            curElm_(std::next(std::begin(elms_),
                  std::distance(std::begin(s.elms_), s.curElm_))) { }
                  */
        {}


        SparseBitmap(SparseBitmap &&s) = default;

        void set_universe(bool universe) {
            this->universe = universe;
        }

        SparseBitmap &operator=(const SparseBitmap &s) {
            this->universe = s.universe;
            elms_.clear();
            elms_.insert(std::end(elms_), std::begin(s.elms_), std::end(s.elms_));

            /*
            curElm_ = std::next(std::begin(elms_),
              std::distance(std::begin(s.elms_), s.curElm_));
              */

            return *this;
        }

        SparseBitmap &operator=(SparseBitmap &&) = default;
        //}}}

        // Misc {{{
        bool empty() const {
            return this->count() == 0;
        }

        void clear() {
            elms_.clear();
        }
        //}}}

        // Accessors {{{
        bool test(id_type id) const {
            auto idx = getIdx(id);
            auto it = findClosest(idx);

            if (it == std::end(elms_)) {
                return false;
            }

            if (it->index() != idx) {
                return false;
            }

            return it->test(getOffs(id));
        }

//        bool intersects(SparseBitmap &rhs) const {
//            if (elms_.empty() && rhs.elms_.empty()) {
//                return false;
//            }
//
//            auto it1 = std::begin(elms_);
//            auto it2 = std::begin(rhs.elms_);
//
//            while (it2 != std::end(rhs.elms_)) {
//                if (it1 == std::end(elms_)) {
//                    return false;
//                }
//
//                if (it1->index() > it2->index()) {
//                    ++it2;
//                } else if (it2->index() > it1->index()) {
//                    ++it1;
//                    // index1 == index2
//                } else {
//                    if (it1->intersects(*it2)) {
//                        return true;
//                    }
//                    ++it1;
//                    ++it2;
//                }
//            }
//            return false;
//        }

        bool singleton() const {
            return elms_.size() == 1 &&
                   elms_.front().count() == 1;
        }

        size_t count() const {
            size_t ret = 0;
            for (auto &elm : elms_) {
                ret += elm.count();
            }
            return ret;
        }
        // }}}

        // Modifiers {{{
        void reset(id_type id) {
            auto idx = getIdx(id);

            auto it = findClosest(idx);

            if (it == std::end(elms_) ||
                it->index() != idx) {
                return;
            }

            it->reset(getOffs(id));

            // If no bits are set, erase the bitset
            if (it->none()) {
                // Advance curElm because we're about to erase this one
                elms_.erase(it);
            }
        }

        void set(id_type id) {
            auto idx = getIdx(id);
            auto it = findClosest(idx);

            // Need to insert at rear?
            if (it == std::end(elms_)) {
                it = elms_.insert(it, node(idx, universe));
            } else if (it->index() != idx) {
                if (it->index() < idx) {
                    it = elms_.insert(++it, node(idx, universe));
                } else {
                    it = elms_.insert(it, node(idx, universe));
                }
            }


            it->set(getOffs(id));
        }

        bool test_and_set(id_type id) {
            auto idx = getIdx(id);
            auto it = findClosest(idx);
            // std::cout << "id: " << id << ", idx: " << idx << std::endl;

            // Need to insert at rear?
            if (it == std::end(elms_)) {
                it = elms_.insert(it, node(idx, universe));
            } else if (it->index() != idx) {
                // std::cout << "it->idx: " << it->index << std::endl;
                if (it->index() < idx) {
                    // std::cout << "InsAfter" << std::endl;
                    it = elms_.insert(++it, node(idx, universe));
                } else {
                    // std::cout << "InsBefore" << std::endl;
                    it = elms_.insert(it, node(idx, universe));
                }
            }


            auto offs = getOffs(id);
            bool ret = !(it->test(offs));
            it->set(offs);
            return ret;
        }

//        bool orWithIntersect(const SparseBitmap &rhs,
//                             const SparseBitmap *intersect) {
//            if (intersect == nullptr) {
//                return operator|=(rhs);
//            }
//
//            // If they are empty nothing changes...
//            if (rhs.empty()) {
//                return false;
//            }
//
//            bool ch = false;
//            auto it1 = std::begin(elms_);
//            auto it2 = std::begin(rhs.elms_);
//            auto it_int = std::begin(intersect->elms_);
//
//            while (it2 != std::end(rhs.elms_)) {
//                while (it_int != std::end(intersect->elms_) &&
//                       it2->index() > it_int->index()) {
//                    ++it_int;
//                }
//
//                if (it_int == std::end(intersect->elms_)) {
//                    break;
//                }
//
//                // Only proceed to check if the two intersect
//                if (it_int->index() > it2->index()) {
//                    ++it2;
//                    continue;
//                }
//
//                node new_node(*it2);
//                new_node &= *it_int;
//
//                // If the intersection of rhs and intersect is empty, move on
//                if (new_node.none()) {
//                    ++it2;
//                    continue;
//                }
//
//                assert(it_int->index() == it2->index());
//                assert(new_node.index() == it2->index());
//
//                // std::cout << "it1->idx: " << it1->index() << std::endl;
//                // std::cout << "it2->idx: " << it2->index() << std::endl;
//                if (it1 == std::end(elms_) || it1->index() > it2->index()) {
//                    elms_.insert(it1, std::move(new_node));
//                    ++it2;
//                    ch = true;
//                } else if (it1->index() == it2->index()) {
//                    ch |= it1->orWithIntersect(*it2, *it_int);
//                    ++it1;
//                    ++it2;
//                } else {
//                    ++it1;
//                }
//            }
//
//            curElm_ = std::begin(elms_);
//
//            return ch;
//        }
//
        SparseBitmap operator-(const SparseBitmap &rhs) const {

            // Now, iterate lhs, and subtract any nodes in lhs also in rhs
            // If they are empty nothing changes...
            if (rhs.empty()) {
                return *this;
            }

            SparseBitmap ret;

            auto it1 = std::begin(ret.elms_);
            auto it2 = std::begin(rhs.elms_);

            if (!universe) {

                while (it1 != std::end(ret.elms_)) {
                    while (it2 != std::end(rhs.elms_) &&
                           it2->index() < it1->index()) {
                        ++it2;
                    }

                    // std::cout << "it1->idx: " << it1->index() << std::endl;
                    // std::cout << "it2->idx: " << it2->index() << std::endl;
                    if (it1->index() == it2->index()) {
                        ret.elms_.push_back(*it1 - *it2);
                    } else {
                        ret.elms_.push_back(*it1);
                    }
                    ++it1;
                }


                return std::move(ret);
            } else {
                while (it1 != std::end(ret.elms_)) {
                    while (it2 != std::end(rhs.elms_) &&
                           it2->index() < it1->index()) {
                        ret.elms_.push_back(node(it2->index(), universe) - *it2);
                        ++it2;
                    }

                    // std::cout << "it1->idx: " << it1->index() << std::endl;
                    // std::cout << "it2->idx: " << it2->index() << std::endl;
                    if (it1->index() == it2->index()) {
                        ret.elms_.push_back(*it1 - *it2);
                    } else {
                        ret.elms_.push_back(*it1);
                    }
                    ++it1;
                }
                while (it2 != std::end(rhs.elms_)) {
                    ret.elms_.push_back(node(it2->index(), universe) - *it2);
                    ++it2;
                }

                return std::move(ret);
            }
        }

        SparseBitmap operator|(const SparseBitmap &rhs) const {

            if (rhs.empty()) {
                return *this;
            }
            SparseBitmap ret = *this;

            auto it1 = std::begin(ret.elms_);
            auto it2 = std::begin(rhs.elms_);

            if (!universe) {

                while (it2 != std::end(rhs.elms_)) {
                    if (it1 == std::end(ret.elms_) || it1->index() > it2->index()) {
                        ret.elms_.insert(it1, *it2);
                        ++it2;
                    } else if (it1->index() == it2->index()) {
                        // Don't do copy if we've already changed
                        *it1 |= *it2;
                        ++it1;
                        ++it2;
                    } else {
                        ++it1;
                    }
                }

                return ret;
            } else {
                while (it2 != std::end(rhs.elms_)) {
                    if (it1 == std::end(ret.elms_) || it1->index() > it2->index()) {
                        ret.elms_.insert(it1, *it2 | node(it2->index(), universe));
                        ++it2;
                    } else if (it1->index() == it2->index()) {
                        // Don't do copy if we've already changed
                        *it1 |= *it2;
                        ++it1;
                        ++it2;
                    } else {
                        ++it1;
                    }
                }
                return ret;
            }

        }

        SparseBitmap operator+(const SparseBitmap &rhs) const {
            return *this | rhs;
        }

        SparseBitmap operator&(const SparseBitmap &rhs) {
            if (elms_.empty() && rhs.elms_.empty()) {
                SparseBitmap ret;
                return ret;
            }
            SparseBitmap ret = *this;

            auto it1 = std::begin(ret.elms_);
            auto it2 = std::begin(rhs.elms_);

            if (!universe) {

                while (it2 != std::end(rhs.elms_)) {
                    if (it1 == std::end(ret.elms_)) {
                        return ret;
                    }

                    if (it1->index() > it2->index()) {
                        ++it2;
                    } else if (it1->index() == it2->index()) {
                        // Don't do copy if we've already changed
                        *it1 &= *it2;

                        if (it1->none()) {
                            auto tmp = it1;
                            ++it1;
                            ret.elms_.erase(tmp);
                        } else {
                            ++it1;
                        }
                        ++it2;
                    } else {
                        auto tmp = it1;
                        ++it1;
                        ret.elms_.erase(tmp);
                    }
                }

                return ret;
            } else {
                while (it2 != std::end(rhs.elms_)) {
                    if (it1 == std::end(ret.elms_)) {

                        break;
                    }

                    if (it1->index() > it2->index()) {
                        ret.elms_.insert(it1, *it2);
                        ++it2;
                    } else if (it1->index() == it2->index()) {
                        // Don't do copy if we've already changed
                        *it1 &= *it2;

                        if (it1->none()) {
                            auto tmp = it1;
                            ++it1;
                            ret.elms_.erase(tmp);
                        } else {
                            ++it1;
                        }
                        ++it2;
                    }
                }
                while (it2 != std::end(rhs.elms_)) {
                    ret.elms_.insert(it1, *it2);
                    ++it2;
                }
                return ret;
            }
        }

        // Operators {{{
        bool operator==(const SparseBitmap &rhs) const {
            auto it1 = std::begin(elms_);
            auto it2 = std::begin(rhs.elms_);

            for (; it1 != std::end(elms_) && it2 != std::end(rhs.elms_); ++it1, ++it2) {
                if (it1->index() != it2->index()) {
                    return false;
                }

                if (!it1->bitmapEqual(*it2)) {
                    return false;
                }
            }

            return it1 == std::end(elms_) && it2 == std::end(rhs.elms_);
        }

        bool operator!=(const SparseBitmap &rhs) const {
            return !(*this == rhs);
        }

        SparseBitmap &operator+=(const SparseBitmap &rhs) {
            return *this |= rhs;
        }

        // Ugh, harder
        SparseBitmap &operator|=(const SparseBitmap &rhs) {
            // If they are empty nothing changes...
            if (rhs.empty()) {
                return *this;
            }

            auto it1 = std::begin(elms_);
            auto it2 = std::begin(rhs.elms_);

            if (!universe) {

                while (it2 != std::end(rhs.elms_)) {
                    // std::cout << "it1->idx: " << it1->index() << std::endl;
                    // std::cout << "it2->idx: " << it2->index() << std::endl;
                    if (it1 == std::end(elms_) || it1->index() > it2->index()) {
                        elms_.insert(it1, *it2);
                        ++it2;
                    } else if (it1->index() == it2->index()) {
                        // Don't do copy if we've already changed
                        *it1 |= *it2;
                        ++it1;
                        ++it2;
                    } else {
                        ++it1;
                    }
                }


                return *this;
            } else {
                while (it2 != std::end(rhs.elms_)) {
                    // std::cout << "it1->idx: " << it1->index() << std::endl;
                    // std::cout << "it2->idx: " << it2->index() << std::endl;
                    if (it1 == std::end(elms_) || it1->index() > it2->index()) {
                        elms_.insert(it1, *it2 | node(it2->index(), universe));
                        ++it2;
                    } else if (it1->index() == it2->index()) {
                        // Don't do copy if we've already changed
                        *it1 |= *it2;
                        ++it1;
                        ++it2;
                    } else {
                        ++it1;
                    }
                }


                return *this;
            }
        }

        const SparseBitmap &operator-=(const SparseBitmap &rhs) const {

            // Now, iterate lhs, and subtract any nodes in lhs also in rhs
            // If they are empty nothing changes...
            if (rhs.empty()) {
                return *this;
            }

            auto it1 = std::begin(elms_);
            auto it2 = std::begin(rhs.elms_);

            if (!universe) {

                while (it1 != std::end(elms_)) {
                    while (it2 != std::end(rhs.elms_) &&
                           it2->index() < it1->index()) {
                        ++it2;
                    }

                    // std::cout << "it1->idx: " << it1->index() << std::endl;
                    // std::cout << "it2->idx: " << it2->index() << std::endl;
                    if (it1->index() == it2->index()) {
                        *it1 -= *it2;
                    }
                    ++it1;
                }
            } else {
                while (it1 != std::end(elms_)) {
                    while (it2 != std::end(rhs.elms_) &&
                           it2->index() < it1->index()) {
                        elms_.push_back(node(it2->index(), universe) - *it2);
                        ++it2;
                    }

                    // std::cout << "it1->idx: " << it1->index() << std::endl;
                    // std::cout << "it2->idx: " << it2->index() << std::endl;
                    if (it1->index() == it2->index()) {
                        *it1 -= *it2;
                    }
                    ++it1;
                }
                while (it2 != std::end(rhs.elms_)) {
                    elms_.push_back(node(it2->index(), universe) - *it2);
                    ++it2;
                }
            }


            return *this;
        }

        SparseBitmap &operator&=(const SparseBitmap &rhs) {
            if (elms_.empty() && rhs.elms_.empty()) {
                return *this;
            }

            auto it1 = std::begin(elms_);
            auto it2 = std::begin(rhs.elms_);

            if (!universe) {

                while (it2 != std::end(rhs.elms_)) {
                    if (it1 == std::end(elms_)) {
                        return *this;
                    }

                    if (it1->index() > it2->index()) {
                        ++it2;
                    } else if (it1->index() == it2->index()) {
                        // Don't do copy if we've already changed
                        *it1 &= *it2;

                        if (it1->none()) {
                            auto tmp = it1;
                            ++it1;
                            elms_.erase(tmp);
                        } else {
                            ++it1;
                        }
                        ++it2;
                    } else {
                        auto tmp = it1;
                        ++it1;
                        elms_.erase(tmp);
                    }
                }
            } else {
                while (it2 != std::end(rhs.elms_)) {
                    if (it1 == std::end(elms_)) {
                        break;
                    }

                    if (it1->index() > it2->index()) {
                        elms_.push_back(*it2 & node(it2->index(), universe));
                        ++it2;
                    } else if (it1->index() == it2->index()) {
                        // Don't do copy if we've already changed
                        *it1 &= *it2;
                        ++it1;
                        ++it2;
                    } else {

                        ++it1;
                    }
                }
                while (it2 != std::end(rhs.elms_)) {
                    elms_.push_back(*it2 & node(it2->index(), universe));
                    ++it2;
                }
            }

            return *this;
        }
        //}}}

        // Hash {{{
        struct hasher {
            size_t operator()(const SparseBitmap &map) const {
                size_t ret = 0;

                for (auto &elm : map.elms_) {
                    ret ^= elm.hash();
                }

                return ret;
            }
        };
        //}}}

        // Iteration {{{
        class iterator :
                public std::iterator<std::forward_iterator_tag, id_type> {
        public:
            // NOTE: We use unsigned long here for size of machine word... works w/ gcc
            int32_t BitsPerShift = 8 * sizeof(unsigned long);  // NOLINT

            explicit iterator(const bitmap_list &bl, bool end) :
                    bl_(bl), it_(std::begin(bl)), end_(end) {
                getFirstBit();
            }

            explicit iterator(const bitmap_list &bl) : bl_(bl), it_(std::begin(bl)) {
                getFirstBit();
            }

            iterator operator++(int) {
                auto tmp = *this;
                advanceBit();
                return tmp;
            }

            iterator &operator++() {
                advanceBit();
                return *this;
            }

            id_type operator*() const {
                // -1 b/c the bsPos_ is 1 indexed
                auto ret = id_type(bsPos_ - 1 +
                                   (bsShift_ * node::BitsPerSet)
                                   + it_->index() * bits_per_field);

                // std::cout << "Returning: " << ret << std::endl;

                return ret;
            }

            bool operator==(const iterator &it) const {
                return end_ == it.end_ ||
                       (it_ == it.it_ &&
                        bsPos_ == it.bsPos_ &&
                        bsShift_ == it.bsShift_);
            }

            bool operator!=(const iterator &it) const {
                return !(*this == it);
            }

        private:
            void getBsVal() {
                bsVal_ = it_->getUl(bsShift_);
                // std::cout << "getBsVal: " << bsVal_ << std::endl;
            }

            void advanceBs() {
                bsShift_++;
            }

            void nextBsVal() {
                advanceBs();
                getBsVal();
            }

            void nextBsPos() {
                int val = __builtin_ffsl(bsVal_);
                assert(val != 0);
                if (val == 8 * sizeof(unsigned long)) {  // NOLINT
                    bsVal_ = 0;
                } else {
                    bsVal_ >>= val;
                }
                /*
                std::cout << "~new bsVal_: " << std::hex << bsVal_ << std::dec << std::endl;
                */
                bsPos_ += val;
            }

            void getFirstBit() {
                if (end_) {
                    return;
                }

                if (bl_.empty()) {
                    end_ = true;
                    return;
                }

                it_ = std::begin(bl_);
                // std::cout << "init it->idx: " << it_->index() << std::endl;

                // Initialize bsVal_;
                getBsVal();

                // std::cout << "init bsVal_: " << bsVal_ << std::endl;

                // Go until we get a non-0 bs
                while (bsVal_ == 0) {
                    nextBsVal();
                }

                /*
                std::cout << "post-inc bsVal_: " << std::hex << bsVal_ << std::dec << std::endl;
                */

                // Okay, now find the first bit
                bsPos_ = 0;
                nextBsPos();
                // std::cout << "init bsPos_: " << bsPos_ << std::endl;
                // std::cout << "init bsVal_: " << bsVal_ << std::endl;
                // std::cout << "init bsShift_: " << bsShift_ << std::endl;
            }

            void advanceBit() {
                if (end_) {
                    return;
                }

                // std::cout << "have bsVal_: " << bsVal_ << std::endl;
                while (bsVal_ == 0) {
                    bsPos_ = 0;
                    // std::cout << "have bsShift_: " << bsShift_ << std::endl;
                    if (bsShift_ == (node::NumBitsets - 1)) {
                        ++it_;
                        // std::cout << "IT advance" << std::endl;
                        if (it_ == std::end(bl_)) {
                            // std::cout << "END" << std::endl;
                            end_ = true;
                            return;
                        }
                        bsShift_ = 0;
                        getBsVal();
                        // std::cout << "BS reset to: " << bsVal_ << std::endl;
                    } else {
                        nextBsVal();
                    }
                }

                // std::cout << "old bsPos_ : " << bsPos_ << std::endl;
                nextBsPos();
                // std::cout << "new bsPos_ : " << bsPos_ << std::endl;
            }

            const bitmap_list &bl_;
            typename bitmap_list::const_iterator it_;

            bool end_ = false;
            uint32_t bsPos_ = 0;
            unsigned long bsVal_;  // NOLINT
            uint32_t bsShift_ = 0;
        };

        typedef iterator const_iterator;

        iterator begin() {
            return iterator(elms_);
        }

        iterator end() {
            return iterator(elms_, true);
        }

        const_iterator begin() const {
            return const_iterator(elms_);
        }

        const_iterator end() const {
            return const_iterator(elms_, true);
        }
        //}}}

        // Print debug {{{
        friend std::ostream &operator<<(std::ostream &os, const SparseBitmap &map) {
            os << "{";
            for (auto val : map) {
                os << " " << val;
            }
            os << " }";
            return os;
        }


        //}}}

    private:
        // Private Methods {{{
        static size_t getIdx(id_type id) {
            return static_cast<size_t>(id) / (bits_per_field);
        }

        static size_t getOffs(id_type id) {
            return static_cast<size_t>(id) % (bits_per_field);
        }

        // Returns the node that contains this id, or the node before it if it isn't
        //   found, or std::end(list) if the element belongs at the front of the list
        typename bitmap_list::iterator findClosest(size_t idx) const {
            if (elms_.empty()) {
                return std::end(elms_);
            }


            auto elm = std::begin(elms_);
            if (elm->index() == idx) {
                return elm;
            } else if (elm->index() > idx) {
                while (elm != std::begin(elms_) &&
                       elm->index() > idx) {
                    --elm;
                }
            } else {
                while (elm != std::end(elms_) &&
                       elm->index() < idx) {
                    ++elm;
                }
            }

            return elm;
        }
        //}}}




        mutable bitmap_list elms_;

        bool universe = false;
        //}}}
        //}}}
    };
//}}}

}  // namespace util

// Hash function for SparseBitmap<>: {{{
namespace std {
    template<>
    struct hash<util::SparseBitmap<>> {
        std::size_t operator()(const util::SparseBitmap<> &map) const {
            return util::SparseBitmap<>::hasher()(map);
        }
    };
}
//}}}

#endif  // UTIL_BITMAP_SPARSE_BITMAP_H_
