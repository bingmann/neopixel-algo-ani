/*******************************************************************************
 * lib/BlinkenAlgorithms/BlinkenAlgorithms/Animation/Sort.hpp
 *
 * Copyright (C) 2018 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the GNU General Public License v3.0
 ******************************************************************************/

#ifndef BLINKENALGORITHMS_ANIMATION_SORT_HEADER
#define BLINKENALGORITHMS_ANIMATION_SORT_HEADER

#include <BlinkenAlgorithms/Color.hpp>
#include <BlinkenAlgorithms/Control.hpp>

#include <cassert>
#include <random>
#include <vector>

#include "TimSort.hpp"
#include "WikiSort.hpp"

namespace BlinkenSort {

using namespace BlinkenAlgorithms;

static const uint16_t black = uint16_t(-1);
static const uint16_t unsigned_negative = uint16_t(32678);

/******************************************************************************/
//! custom struct for array items, which allows detailed counting of comparisons

class Item
{
public:
    typedef uint16_t value_type;

public:
    value_type value_;

public:
    Item() { }

    explicit Item(const value_type& d) : value_(d) { OnAccess(this); }

    Item(const Item& v) : value_(v.value_) {
        OnAccess(this);
    }

    Item(Item&& v) : value_(v.value_) {
        v.value_ = black;
        OnAccess(this);
    }

    Item& operator = (const Item& a) {
        value_ = a.value_;
        OnAccess(this);
        return *this;
    }

    Item& operator = (Item&& a) {
        value_ = a.value_;
        a.value_ = black;
        OnAccess(this);
        return *this;
    }

    //! Returns value_
    value_type value() const {
        OnAccess(this, /* with_delay */ true);
        return value_;
    }

    Item& operator ++ (int) {
        value_++;
        OnAccess(this);
        return *this;
    }

    Item& operator -- (int) {
        value_--;
        OnAccess(this);
        return *this;
    }

    // *** bypass delay and updates

    Item& SetNoDelay(const value_type& d) {
        value_ = d;
        OnAccess(this, /* with_delay */ false);
        return *this;
    }

    Item& SetNoDelay(const Item& a) {
        value_ = a.value_;
        OnAccess(this, /* with_delay */ false);
        return *this;
    }

    void SwapNoDelay(Item& a) {
        Item tmp;
        tmp.SetNoDelay(a);
        a.SetNoDelay(*this);
        SetNoDelay(tmp);
    }

    // *** comparisons

    bool operator == (const Item& v) const {
        OnComparison(*this, v);
        return (value_ == v.value_);
    }

    bool operator != (const Item& v) const {
        OnComparison(*this, v);
        return (value_ != v.value_);
    }

    bool operator < (const Item& v) const {
        OnComparison(*this, v);
        return (value_ < v.value_);
    }

    bool operator <= (const Item& v) const {
        OnComparison(*this, v);
        return (value_ <= v.value_);
    }

    bool operator > (const Item& v) const {
        OnComparison(*this, v);
        return (value_ > v.value_);
    }

    bool operator >= (const Item& v) const {
        OnComparison(*this, v);
        return (value_ >= v.value_);
    }

    // ternary comparison which counts just one
    int cmp(const Item& v) const {
        OnComparison(*this, v);
        return (value_ == v.value_ ? 0 : value_ < v.value_ ? -1 : +1);
    }

    // *** comparisons without sound, counting or delay

    bool equal_direct(const Item& v) const { return (value_ == v.value_); }

    bool less_direct(const Item& v) const { return (value_ < v.value_); }

    bool greater_direct(const Item& v) const { return (value_ > v.value_); }

    // *** access and comparison collectors

    static void OnAccess(const Item* a, bool with_delay = true);
    static void OnComparison(const Item&, const Item&);

    static void IncrementCounter();
};

using SortFunctionType = void (*)(Item * A, size_t n);

class SortAnimationBase
{
public:
    virtual void OnAccess(const Item* a, bool with_delay) = 0;
    virtual void OnComparison(const Item* a, const Item* b) = 0;
    virtual void IncrementCounter() = 0;
};

static SortAnimationBase* sort_animation_hook = nullptr;

// callbacks
static void (* SoundAccessHook)(size_t i) = nullptr;
static void (* DelayHook)() = nullptr;
static void (* AlgorithmNameHook)(const char* name) = nullptr;
static void (* ComparisonCountHook)(size_t count) = nullptr;
static unsigned intensity_flash_high = 2;

void Item::OnAccess(const Item* a, bool with_delay) {
    if (sort_animation_hook)
        sort_animation_hook->OnAccess(a, with_delay);
    if (SoundAccessHook)
        SoundAccessHook(a->value_);
}

void Item::OnComparison(const Item& a, const Item& b) {
    if (sort_animation_hook) {
        sort_animation_hook->OnComparison(&a, &b);
    }
    if (SoundAccessHook) {
        SoundAccessHook(a.value_);
        SoundAccessHook(b.value_);
    }
}

void Item::IncrementCounter() {
    if (sort_animation_hook) {
        sort_animation_hook->IncrementCounter();
    }
}

/******************************************************************************/
// Sorting Algorithms

size_t array_size;
std::vector<Item> array;

using std::swap;

/******************************************************************************/
// Selection Sort

void SelectionSort(Item* A, size_t n) {
    for (size_t i = 0; i < n - 1; ++i) {
        size_t j_min = i;
        for (size_t j = i + 1; j < n; ++j) {
            if (A[j] < A[j_min]) {
                j_min = j;
            }
        }
        swap(A[i], A[j_min]);
    }
}

/******************************************************************************/
// Insertion Sort

void InsertionSort(Item* A, size_t n) {
    for (size_t i = 1; i < n && !g_terminate; ++i) {
        Item key = A[i];

        ssize_t j = i - 1;
        while (j >= 0 && A[j] > key) {
            swap(A[j], A[j + 1]);
            j--;
        }
    }
}

/******************************************************************************/
// Bubble Sort

void BubbleSort(Item* A, size_t n) {
    for (size_t i = 0; i < n - 1; ++i) {
        for (size_t j = 0; j < n - 1 - i; ++j) {
            if (A[j] > A[j + 1]) {
                swap(A[j], A[j + 1]);
            }
        }
    }
}

/******************************************************************************/
// Cocktail Shaker Sort

void CocktailShakerSort(Item* A, size_t n) {
    size_t lo = 0, hi = n - 1, mov = lo;

    while (lo < hi) {
        for (size_t i = hi; i > lo; --i) {
            if (A[i - 1] > A[i]) {
                swap(A[i - 1], A[i]);
                mov = i;
            }
        }

        lo = mov;

        for (size_t i = lo; i < hi; ++i) {
            if (A[i] > A[i + 1]) {
                swap(A[i], A[i + 1]);
                mov = i;
            }
        }

        hi = mov;
    }
}

/******************************************************************************/
// QuickSort

enum QuickSortPivotType {
    PIVOT_FIRST,   //!< always select first item
    PIVOT_LAST,    //!< always select last item
    PIVOT_MID,     //!< always select middle item
    PIVOT_RANDOM,  //!< always pick a random item
    PIVOT_MEDIAN3, //!< select median of three
    PIVOT_SIZE
};

QuickSortPivotType g_quicksort_pivot = PIVOT_FIRST;

// pivot selection method
ssize_t QuickSortSelectPivot(Item* A, ssize_t lo, ssize_t hi) {
    if (g_quicksort_pivot == PIVOT_FIRST)
        return lo;

    if (g_quicksort_pivot == PIVOT_LAST)
        return hi - 1;

    if (g_quicksort_pivot == PIVOT_MID)
        return (lo + hi) / 2;

    if (g_quicksort_pivot == PIVOT_RANDOM)
        return lo + (rand() % (hi - lo));

    if (g_quicksort_pivot == PIVOT_MEDIAN3) {
        ssize_t mid = (lo + hi) / 2;

        // cases if two are equal
        if (A[lo] == A[mid])
            return lo;
        if (A[lo] == A[hi - 1] || A[mid] == A[hi - 1])
            return hi - 1;

        // cases if three are different
        return A[lo] < A[mid]
               ? (A[mid] < A[hi - 1] ? mid
                  : (A[lo] < A[hi - 1] ? hi - 1 : lo))
               : (A[mid] > A[hi - 1] ? mid
                  : (A[lo] < A[hi - 1] ? lo : hi - 1));
    }

    return lo;
}

/******************************************************************************/
// Quick Sort LR (pointers left and right, Hoare's partition schema)

void QuickSortLR(Item* A, ssize_t lo, ssize_t hi) {
    if (g_terminate)
        return;

    ssize_t p = QuickSortSelectPivot(A, lo, hi + 1);

    ssize_t i = lo, j = hi;

    while (i <= j && !g_terminate) {
        while (A[i] < A[p])
            i++;
        while (A[j] > A[p])
            j--;
        if (i <= j) {
            swap(A[i], A[j]);
            // follow pivot if it is swapped
            p = (p == i ? j : p == j ? i : p);
            i++, j--;
        }
    }

    if (lo < j)
        QuickSortLR(A, lo, j);
    if (i < hi)
        QuickSortLR(A, i, hi);
}

void QuickSortLR(Item* A, size_t n) {
    g_quicksort_pivot = (QuickSortPivotType)random(PIVOT_SIZE);
    QuickSortLR(A, 0, n - 1);
}

/******************************************************************************/
// Quick Sort LL (Lomuto partition scheme, two pointers at left, pivot is moved
// to the right) (code by Timo Bingmann, based on CLRS' 3rd edition)

ssize_t PartitionLL(Item* A, ssize_t lo, ssize_t hi) {
    // pick pivot and move to back
    size_t p = QuickSortSelectPivot(A, lo, hi + 1);
    swap(A[p], A[hi]);

    Item& pivot = A[hi];

    ssize_t i = lo;

    for (ssize_t j = lo; j < hi; ++j) {
        if (A[j] < pivot) {
            swap(A[i], A[j]);
            ++i;
        }
    }

    swap(A[i], A[hi]);

    return i;
}

void QuickSortLL(Item* A, ssize_t lo, ssize_t hi) {
    if (lo < hi) {
        ssize_t mid = PartitionLL(A, lo, hi);

        QuickSortLL(A, lo, mid - 1);
        QuickSortLL(A, mid + 1, hi);
    }
}

void QuickSortLL(Item* A, size_t n) {
    g_quicksort_pivot = (QuickSortPivotType)random(PIVOT_SIZE);
    QuickSortLL(A, 0, n - 1);
}

/******************************************************************************/
// Dual-Pivot Quick Sort (code by Yaroslavskiy via Sebastian Wild)

void QuickSortDualPivotYaroslavskiy(Item* A, int left, int right) {
    if (right > left) {
        if (A[left] > A[right]) {
            swap(A[left], A[right]);
        }

        const Item p = A[left];
        const Item q = A[right];

        ssize_t l = left + 1;
        ssize_t g = right - 1;
        ssize_t k = l;

        while (k <= g) {
            if (A[k] < p) {
                swap(A[k], A[l]);
                ++l;
            }
            else if (A[k] >= q) {
                while (A[g] > q && k < g)
                    --g;
                swap(A[k], A[g]);
                --g;

                if (A[k] < p) {
                    swap(A[k], A[l]);
                    ++l;
                }
            }
            ++k;
        }
        --l;
        ++g;
        swap(A[left], A[l]);
        swap(A[right], A[g]);

        QuickSortDualPivotYaroslavskiy(A, left, l - 1);
        QuickSortDualPivotYaroslavskiy(A, l + 1, g - 1);
        QuickSortDualPivotYaroslavskiy(A, g + 1, right);
    }
}

void QuickSortDualPivot(Item* A, size_t n) {
    return QuickSortDualPivotYaroslavskiy(A, 0, n - 1);
}

/******************************************************************************/
// Merge Sort (out-of-place with sentinels) (code by myself, Timo Bingmann)

void Merge(Item* A, size_t lo, size_t mid, size_t hi) {
    // allocate output
    Item out[hi - lo];

    // merge
    size_t i = lo, j = mid, o = 0; // first and second halves
    while (i < mid && j < hi && !g_terminate) {
        if (A[i] < A[j])
            out[o++] = std::move(A[i++]);
        else
            out[o++] = std::move(A[j++]);
    }

    // copy rest
    while (i < mid)
        out[o++] = std::move(A[i++]);
    while (j < hi)
        out[o++] = std::move(A[j++]);

    // copy back
    for (i = 0; i < hi - lo && !g_terminate; ++i)
        A[lo + i] = std::move(out[i]);
}

void MergeSort(Item* A, size_t lo, size_t hi) {
    if (g_terminate)
        return;

    if (lo + 1 < hi) {
        size_t mid = (lo + hi) / 2;

        MergeSort(A, lo, mid);
        MergeSort(A, mid, hi);

        Merge(A, lo, mid, hi);
    }
}

void MergeSort(Item* A, size_t n) {
    return MergeSort(A, 0, n);
}

void MergeSortIterative(Item* A, size_t n) {
    for (size_t s = 1; s < n; s *= 2) {
        for (size_t i = 0; i + s < n; i += 2 * s) {
            Merge(A, i, i + s, std::min(i + 2 * s, n));
        }
    }
}

/******************************************************************************/
// Shell's Sort

void ShellSort(Item* A, size_t n) {
    size_t incs[16] = {
        1391376, 463792, 198768, 86961, 33936, 13776, 4592, 1968,
        861, 336, 112, 48, 21, 7, 3, 1
    };

    for (size_t k = 0; k < 16; k++) {
        for (size_t h = incs[k], i = h; i < n; i++) {
            Item v = A[i];
            size_t j = i;

            while (j >= h && A[j - h] > v) {
                A[j] = A[j - h];
                j -= h;
            }

            A[j] = v;
        }
    }
}

/******************************************************************************/
// Heap Sort

bool isPowerOfTwo(size_t x) {
    return ((x != 0) && !(x & (x - 1)));
}

uint32_t prevPowerOfTwo(uint32_t x) {
    x |= x >> 1, x |= x >> 2, x |= x >> 4, x |= x >> 8, x |= x >> 16;
    return x - (x >> 1);
}

int largestPowerOfTwoLessThan(int n) {
    int k = 1;
    while (k < n)
        k = k << 1;
    return k >> 1;
}

void HeapSort(Item* A, size_t n) {
    size_t i = n / 2;

    while (!g_terminate) {
        if (i > 0) {
            // build heap, sift A[i] down the heap
            i--;
        }
        else {
            // pop largest element from heap: swap front to back, and sift
            // front A[0] down the heap
            n--;
            if (n == 0)
                return;
            swap(A[0], A[n]);
        }

        size_t parent = i;
        size_t child = i * 2 + 1;

        // sift operation - push the value_ of A[i] down the heap
        while (child < n) {
            if (child + 1 < n && A[child + 1] > A[child]) {
                child++;
            }
            if (A[child] > A[parent]) {
                swap(A[parent], A[child]);
                parent = child;
                child = parent * 2 + 1;
            }
            else {
                break;
            }
        }
    }
}

/******************************************************************************/
// Cycle Sort (adapted from http://en.wikipedia.org/wiki/Cycle_sort)

void CycleSort(Item* A, size_t n) {
    size_t cycleStart = 0;
    size_t rank = 0;

    // Loop through the array to find cycles to rotate.
    for (cycleStart = 0; cycleStart + 1 < n; ++cycleStart) {
        Item& item = A[cycleStart];

        do {
            // Find where to put the item.
            rank = cycleStart;
            for (size_t i = cycleStart + 1; i < n; ++i) {
                // special: compare and count but don't flash
                if (A[i].less_direct(item))
                    rank++;
                A[i].IncrementCounter();
            }

            // If the item is already there, this is a 1-cycle.
            if (rank == cycleStart) {
                break;
            }

            // Otherwise, put the item after any duplicates.
            while (item == A[rank])
                rank++;

            // Put item into right place and colorize
            swap(A[rank], A[cycleStart]);

            // Continue for rest of the cycle.
        } while (rank != cycleStart);
    }
}

/******************************************************************************/
// Radix Sort (counting sort, most significant digit (MSD) first, in-place
// redistribute) (code by myself, Timo Bingmann)

void RadixSortMSD(Item* A, size_t n, size_t lo, size_t hi, size_t depth) {
    // radix and base calculations
    const unsigned int RADIX = 4;

    unsigned int pmax = floor(log(n) / log(RADIX));
    size_t base = pow(RADIX, pmax - depth);

    // count digits
    std::vector<size_t> count(RADIX, 0);

    for (size_t i = lo; i < hi; ++i) {
        size_t r = A[i].value() / base % RADIX;
        count[r]++;
    }

    // inclusive prefix sum
    std::vector<size_t> bkt(RADIX, 0);
    std::partial_sum(count.begin(), count.end(), bkt.begin());

    // reorder items in-place by walking cycles
    for (size_t i = 0, j; i < (hi - lo); ) {
        while ((j = --bkt[(A[lo + i].value() / base % RADIX)]) > i) {
            swap(A[lo + i], A[lo + j]);
        }
        i += count[(A[lo + i].value() / base % RADIX)];
    }

    // no more depth to sort?
    if (depth + 1 > pmax)
        return;

    // recurse on buckets
    size_t sum = lo;
    for (size_t i = 0; i < RADIX; ++i) {
        if (count[i] > 1)
            RadixSortMSD(A, n, sum, sum + count[i], depth + 1);
        sum += count[i];
    }
}

void RadixSortMSD(Item* A, size_t n) {
    return RadixSortMSD(A, n, 0, n, 0);
}

/******************************************************************************/
// Radix Sort (counting sort, least significant digit (LSD) first, out-of-place
// redistribute) (code by myself, Timo Bingmann)

void RadixSortLSD(Item* A, size_t n) {
    // radix and base calculations
    const unsigned int RADIX = 4;

    unsigned int pmax = ceil(log(n) / log(RADIX));

    for (unsigned int p = 0; p < pmax; ++p) {
        size_t base = pow(RADIX, p);

        // count digits and copy data
        std::vector<size_t> count(RADIX, 0);
        std::vector<Item> copy(n);

        for (size_t i = 0; i < n; ++i) {
            size_t r = (copy[i] = A[i]).value() / base % RADIX;
            assert(r < RADIX);
            count[r]++;
        }

        // exclusive prefix sum
        std::vector<size_t> bkt(RADIX + 1, 0);
        std::partial_sum(count.begin(), count.end(), bkt.begin() + 1);

        // redistribute items back into array (stable)
        for (size_t i = 0; i < n; ++i) {
            size_t r = copy[i].value() / base % RADIX;
            A[bkt[r]++] = copy[i];
        }
    }
}

/******************************************************************************/

void StdSort(Item* A, size_t n) {
    std::sort(A, A + n);
}

void StdStableSort(Item* A, size_t n) {
    std::stable_sort(A, A + n);
}

/******************************************************************************/

void WikiSort(Item* A, size_t n) {
    WikiSortNS::Sort(A, A + n, std::less<Item>());
}

/******************************************************************************/

void TimSort(Item* A, size_t n) {
    TimSortNS::timsort(A, A + n);
}

/******************************************************************************/
// BozoSort

void BozoSort(Item* A, size_t n) {
    unsigned long ts = millis() + 20000;
    while (millis() < ts) {
        // swap two random items
        swap(A[random(n)], A[random(n)]);
        // swap two random items
        swap(A[random(n)], A[random(n)]);
        // swap two random items
        swap(A[random(n)], A[random(n)]);
        // swap two random items
        swap(A[random(n)], A[random(n)]);
    }
}

/******************************************************************************/

template <typename LEDStrip>
class SortAnimation : public SortAnimationBase
{
public:
    SortAnimation(LEDStrip& strip, int32_t delay_time = 1000)
        : strip_(strip) {
        // hook sorting animation callbacks
        sort_animation_hook = this;

        // set strip size
        array_size = strip_.size();
        array.resize(array_size);
        intensity_last = strip.intensity();

        frame_drop_ = 0;
        delay_time_ = 0;
        set_delay_time(delay_time);

        enable_count_ = true;
    }

    ~SortAnimation() {
        // free array
        std::vector<Item>().swap(array);
    }

    void array_randomize() {
        for (uint32_t i = 0; i < array_size; ++i) {
            array[i].SetNoDelay(i);
        }
        for (uint32_t i = 0; i < array_size; ++i) {
            uint32_t j = random(array_size);
            array[i].SwapNoDelay(array[j]);
        }
    }

    void array_black() {
        for (uint32_t i = 0; i < array_size; ++i) {
            array[i].SetNoDelay(black);
        }
    }

    void array_check() {
        // for (size_t i = 1; i < array_size; ++i) {
        //     if (array[i - 1] > array[i]) {
        //         array[i - 1] = Item(black);
        //     }
        // }
        for (size_t i = 0; i < array_size; ++i) {
            if (array[i] != Item(i)) {
                array[i] = Item(black);
            }
        }
    }

    unsigned intensity_last = 0;

    void OnAccess(const Item* a, bool with_delay) override {
        if (a < array.data() || a >= array.data() + array_size)
            return;
        flash(a - array.data(), with_delay);
    }

    size_t counter_value = 0;

    void IncrementCounter() {
        if (enable_count_)
            ++counter_value;
        if (ComparisonCountHook)
            ComparisonCountHook(counter_value);
    }

    void OnComparison(const Item* a, const Item* b) override {
        IncrementCounter();
        if (a >= array.data() && a < array.data() + array_size &&
            b >= array.data() && b < array.data() + array_size)
            flash(a - array.data(), b - array.data(), /* with_delay */ true);
        else if (a >= array.data() && a < array.data() + array_size)
            flash(a - array.data(), /* with_delay */ true);
        else if (b >= array.data() && b < array.data() + array_size)
            flash(b - array.data(), /* with_delay */ true);
    }

    void set_delay_time(int32_t delay_time) {
        pflush();

        delay_time_ = delay_time;

        if (delay_time_ < 0) {
            frame_drop_ = -delay_time_;
            frame_buffer_pos_ = frame_drop_ - 1;
        }
        else {
            frame_drop_ = 0;
            frame_buffer_pos_ = 0;
        }
    }

    void set_enable_count(bool enable_count) {
        enable_count_ = enable_count;
    }

    void yield_delay(int32_t delay_time) {

        if (delay_time > 0) {
            int32_t remain = delay_time;
            while (remain > 100000) {
                delay_micros(100000);
                remain -= 100000;
            }
            delay_micros(remain);
        }
        if (DelayHook)
            DelayHook();

        if (intensity_last != strip_.intensity()) {
            intensity_last = strip_.intensity();
            for (size_t i = 0; i < array_size; ++i) {
                flash_low(i);
            }
        }
    }

    void yield_delay() {
        if (delay_time_ < 0) {
            yield_delay(delay_time_);
        }
        else {
            // apply global delay factor
            yield_delay(delay_time_ * g_delay_factor / 1000);
        }
    }

    uint16_t value_to_hue(size_t i) { return i * HSV_HUE_MAX / array_size; }

    void flash_low(size_t i) {
        if (array[i].value_ == black)
            strip_.setPixel(i, 0);
        else
            strip_.setPixel(
                i, HSVColor(value_to_hue(array[i].value_), 255,
                            strip_.intensity()));
    }

    void flash_high(size_t i) {
        size_t intensity_high = strip_.intensity();
        intensity_high = (intensity_high * intensity_flash_high) / 100;
        if (intensity_high > 255)
            intensity_high = 255;

        if (array[i].value_ == black) {
            strip_.setPixel(i, Color(intensity_high));
        }
        else {
            Color c = HSVColor(
                value_to_hue(array[i].value_), 255, intensity_high);
            c.white = intensity_high;
            strip_.setPixel(i, c);
        }
    }

    size_t frame_buffer_[256] = { 0 };
    size_t frame_buffer_pos_ = 0;
    size_t frame_drop_ = 0;

    void flash_low_buffer(size_t i) {
        frame_buffer_[frame_buffer_pos_] = i;

        if (frame_buffer_pos_ == 0) {
            if (!strip_.busy()) {
                strip_.show();
            }

            // reset pixels in this frame_buffer_pos_
            for (size_t k = 0; k < frame_drop_; ++k) {
                if (frame_buffer_[k] < array_size)
                    flash_low(frame_buffer_[k]);
            }

            frame_buffer_pos_ = frame_drop_ - 1;
            yield_delay();
        }
        else {
            --frame_buffer_pos_;
        }
    }

    void flash(size_t i, bool with_delay = true) {
        if (!with_delay)
            return flash_low(i);

        if (frame_drop_ == 0) {
            flash_high(i);

            if (!strip_.busy())
                strip_.show();

            yield_delay();

            flash_low(i);
        }
        else {
            flash_high(i);
            flash_low_buffer(i);
        }
    }

    void flash(size_t i, size_t j, bool with_delay = true) {
        if (!with_delay)
            return flash_low(i), flash_low(j);

        if (frame_drop_ == 0) {
            flash_high(i), flash_high(j);

            if (!strip_.busy())
                strip_.show();

            yield_delay();

            flash_low(j), flash_low(i);
        }
        else {
            flash_high(i), flash_high(j);

            flash_low_buffer(j), flash_low_buffer(i);
        }
    }

    void pflush() {
        // reset pixels in this frame_buffer_pos_
        for (size_t j = 0; j < frame_drop_; ++j) {
            if (frame_buffer_[j] < array_size)
                flash_low(frame_buffer_[j]);
        }

        frame_buffer_pos_ = frame_drop_ - 1;
        yield_delay();

        strip_.show();
    }

protected:
    LEDStrip& strip_;

    //! user given delay time.
    int32_t delay_time_;

    //! whether to count comparisons
    bool enable_count_;
};

template <typename LEDStrip>
void RunSort(LEDStrip& strip, const char* algo_name,
             void (*sort_function)(Item* A, size_t n),
             int32_t delay_time = 10000) {

    uint32_t ts = millis();

    SortAnimation<LEDStrip> ani(strip, delay_time);
    if (AlgorithmNameHook)
        AlgorithmNameHook(algo_name);
    ani.array_randomize();
    sort_function(array.data(), array_size);

    static double total_time = 0, total_count = 0;
    total_time += (millis() - ts) / 1000.0;
    total_count += 1;

    printf("%s running time: %.2f fix delay_time %.2f %.2f\n",
           algo_name, (millis() - ts) / 1000.0,
           41.0 / ((millis() - ts) / 1000.0) * delay_time,
           41.0 / (total_time / total_count) * delay_time);

    ts = millis();
    ani.set_delay_time(-4);
    ani.set_enable_count(false);
    ani.array_check();
    ani.pflush();
    // printf("%s check time: %.2f\n", algo_name, (millis() - ts) / 1000.0);
    ani.yield_delay(2000000);
}

/******************************************************************************/

} // namespace BlinkenSort

#endif // !BLINKENALGORITHMS_ANIMATION_SORT_HEADER

/******************************************************************************/
