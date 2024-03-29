//
// Created by easy on 27.10.23.
//

#ifndef AXVECTOR_AXVECTOR_H
#define AXVECTOR_AXVECTOR_H

#include <stdbool.h>
#include <stdint.h>

/*
    axvector is a dynamic vector/array library that has some functional programming concepts and some useful utility
    functions built in.

    Some functions use a comparator. The default comparator compares the addresses of items, but a custom
    comparator can be given and shall conform to the C standard library's comparison function specifications.

    A destructor function may also be supplied. There is no default destructor. The destructor will be called on items
    that are irrevocably removed from the vector. Its prototype is void (*)(void *), like the free() function.

    To provide built-in bookkeeping, the vector also has storage for a context. This is simply a void *. The context is
    not used by the vector itself and is exclusively controlled by the user.

    axvector supports negative indexing: -1 is the last item, -2 the penultimate one etc. All functions that take
    two indices to signify a section treat the first index inclusively and the second index exclusively.
*/
typedef struct axvector axvector;

/*
    Snapshots capture the current state of an axvector. A snapshot consists of an index initialised to 0,
    the current length and a pointer to the first item.

    Snapshots are the quickest way of iterating a vector, but they do not update their state, so the programmer must
    be wary of not invalidating the snapshot.

    Consider using the provided higher-order functions instead if this is not a very tight loop.
*/
typedef struct axvsnap {
    int64_t i;         // index (initialised to 0), increment this in your loop
    int64_t len;       // length of vector at time of taking axv_snapshot, best not to change manually
    void **vec;        // pointer to first element at time of taking axv_snapshot, best not to change manually
} axvsnap;


/**
 * Create axvector with starting capacity.
 * @param size Capacity.
 * @return New axvector or NULL if OOM.
 */
axvector *axv_sizedNew(uint64_t size);
/**
 * Create axvector with default capacity.
 * @return New axvector or NULL if OOM.
 */
axvector *axv_new(void);
/**
 * If destructor is set, call destructor on all items. Destroy axvector and free memory.
 * @return Context.
 */
void *axv_destroy(axvector *v);
/**
 * Increment reference counter. Not thread-safe.
 * @return Self.
 */
axvector *axv_iref(axvector *v);
/**
 * Decrement reference counter. Not thread-safe. Vector is destroyed if no references are held thereafter.
 * @return True iff vector has been destroyed.
 */
bool axv_dref(axvector *v);
/**
 * Amount of references held on this vector.
 * @return Reference count.
 */
int64_t axv_refs(axvector *v);
/**
 * Create a snapshot of this vector. Snapshots are not heap-allocated, thus must not be freed.
 * @return Snapshot.
 */
axvsnap axv_snapshot(axvector *v);
/**
 * Push an item at the end of the vector. Vector is automatically resized if need be.
 * @param val Item.
 * @return True iff OOM during resize operation. Item is not pushed in this case.
 */
bool axv_push(axvector *v, void *val);
/**
 * Pop off (remove) the last item.
 * @return The last item.
 */
void *axv_pop(axvector *v);
// get topmost item without removing
/**
 * Get last item without removing it.
 * @return The last item.
 */
void *axv_top(axvector *v);
/**
 * Number of items in this vector. Consider using higher-order functions or snapshots for iteration instead.
 * @return Length of vector.
 */
int64_t axv_len(axvector *v);
/**
 * Index vector and return item.
 * @param index May be negative.
 * @return Item at index or NULL if index out of range.
 */
void *axv_at(axvector *v, int64_t index);
/**
 * Replace item at index with a new item.
 * @param index May be negative.
 * @param val The new item.
 * @return True iff index out of range.
 */
bool axv_set(axvector *v, int64_t index, void *val);
/**
 * Swap two items.
 * @param index1 May be negative.
 * @param index2 May be negative.
 * @return True iff any index out of range.
 */
bool axv_swap(axvector *v, int64_t index1, int64_t index2);
/**
 * Reverse all items in-place.
 * @return Self.
 */
axvector *axv_reverse(axvector *v);
/**
 * Reverse items in some section in-place.
 * @param index1 Beginning of section. May be negative. Inclusive.
 * @param index2 End of section. May be negative. Exclusive.
 * @return
 */
bool axv_reverseSection(axvector *v, int64_t index1, int64_t index2);
/**
 * Rotate items by k places to the right. Negative k rotates to the left. In-place. O(n).
 * @param k Rotation value.
 * @return Self.
 */
axvector *axv_rotate(axvector *v, int64_t k);
/**
 * Shift all items toward or away from some anchor point. If n is positive, all items starting at the anchor point
 * are shifted n places to the right and the vector is resized as needed. The resulting gap in the vector is
 * filled with zeroes. If n is negative, the first n items starting at the anchor point are removed from the vector.
 * Subsequent items are then shifted toward the anchor point. If a destructor is set, it is called upon all removed
 * items.
 * @param index The anchor point. May be negative. Inclusive.
 * @param n Positive to reserve space amidst the items. Negative to remove items and collapse the vector.
 * @return True iff OOM during resize operation. Vector is unmodified in this case.
 */
bool axv_shift(axvector *v, int64_t index, int64_t n);
/**
 * Remove the last n items. If a destructor is set, it is called upon all removed items.
 * @param n Number of items to remove.
 * @return Self.
 */
axvector *axv_discard(axvector *v, uint64_t n);
/**
 * Removes all items. If a destructor is set, it is called upon every item.
 * @return Self.
 */
axvector *axv_clear(axvector *v);
/**
 * Create a shallow copy of a vector. The copy contains all items of the original and is created with the same
 * capacity. The comparator and context are copied, the destructor and reference counter are not.
 * @return New axvector or NULL if OOM.
 */
axvector *axv_copy(axvector *v);
/**
 * All items of the second vector are moved to the end of the first vector, thereby clearing the second vector.
 * If both vectors are the same, nothing is done. The first vector is resized as needed.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return True iff OOM during resize operation.
 */
bool axv_extend(axvector *v1, axvector *v2);
/**
 * All items of the second vector are copied to the end of the first vector. No changes are done to the second
 * vector. The vectors may be the same. The first vector is resized as needed.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return True iff OOM during resize operation.
 */
bool axv_concat(axvector *v1, axvector *v2);
/**
 * Create a shallow copy of a vector. The copy contains all items of the original which are in the specified slice.
 * The copy is created with capacity equal to the number of items copied or 1 if the slice is empty. The comparator
 * and context are copied, the destructor and reference counter are not.
 * @param index1 Beginning of slice. May be negative. Inclusive.
 * @param index2 End of slice. May be negative. Exclusive.
 * @return New axvector or NULL if OOM.
 */
axvector *axv_slice(axvector *v, int64_t index1, int64_t index2);
/**
 * Create a shallow copy of a vector. The copy contains all items of the original which are in the specified slice
 * in reverse order. The copy is created with capacity equal to the number of items copied or 1 if the slice is empty.
 * The comparator and context are copied, the destructor and reference counter are not.
 * @param index1 Beginning of slice. May be negative. Inclusive.
 * @param index2 End of slice. May be negative. Exclusive.
 * @return New axvector or NULL if OOM.
 */
axvector *axv_rslice(axvector *v, int64_t index1, int64_t index2);
/**
 * Set capacity of vector to some value. If the new capacity is less than the length of the vector and a destructor
 * is set, the destructor will be called on all excess items. This is done even if the resize operation itself fails.
 * @param size New capacity.
 * @return True iff OOM.
 */
bool axv_resize(axvector *v, uint64_t size);
/**
 * If a destructor is set, it is called on the argument. Otherwise this function does nothing.
 * @param val Value to call destructor on.
 * @return Self.
 */
axvector *axv_destroyItem(axvector *v, void *val);
/**
 * Search the greatest item according to the comparator using forward linear search.
 * @return The greatest item or NULL if the vector is empty.
 */
void *axv_max(axvector *v);
/**
 * Search the least item according to the comparator using forward linear search.
 * @return The least item or NULL if the vector is empty.
 */
void *axv_min(axvector *v);
/**
 * Let f be a predicate taking (item in vector, optional argument).
 * Check if any item x contained in the vector satisfies f(x, arg) and stop at the first occurrence
 * of such satisfaction. Items are checked linearly from first to last.
 * @param f Some predicate to apply to the vector.
 * @param arg An optional argument passed to the predicate.
 * @return True iff any item satisfies the predicate.
 */
bool axv_any(axvector *v, bool (*f)(const void *, void *), void *arg);
/**
 * Let f be a predicate taking (item in vector, optional argument).
 * Check if all items x contained in the vector satisfy f(x, arg) and stop at the first occurrence
 * of no such satisfaction. Items are checked linearly from first to last.
 * @param f Some predicate to apply to the vector.
 * @param arg An optional argument passed to the predicate.
 * @return True iff all items satisfy the predicate.
 */
bool axv_all(axvector *v, bool (*f)(const void *, void *), void *arg);
/**
 * Number of items in this vector comparing equal to the given argument according to the comparator.
 * @param val The value all items are to be compared against.
 * @return The resulting count.
 */
int64_t axv_count(axvector *v, void *val);
/**
 * Using the first vector's comparator, compare all items of the first and second vector. Comparison stops
 * prematurely if any unequal item is found. Comparisons are done linearly from first to last item.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return True iff the vectors have the same length and all items compare equal.
 */
bool axv_compare(axvector *v1, axvector *v2);
/**
 * Apply some function f on every item. Maps are done linearly from first to last item.
 * @param f Function taking an item and returning whatever to overwrite its spot in the vector with.
 * @return Self.
 */
axvector *axv_map(axvector *v, void *(*f)(void *));
/**
 * Let f be a predicate taking (item in vector, optional argument).
 * Keep all items x in the vector that satisfy f(x, arg), remove all those that don't, and close the
 * resulting gaps by contracting the space between all remaining items, thus preserving the relative order
 * of the remaining items. If a destructor is set, it is called upon all removed items. The filter is applied
 * linearly from first to last item. O(n).
 * @param f Some predicate to filter the vector.
 * @param arg An optional argument passed to the predicate.
 * @return Self.
 */
axvector *axv_filter(axvector *v, bool (*f)(const void *, void *), void *arg);
/**
 * Let f be a predicate taking (item in vector, optional argument).
 * Keep all items x in the vector that satisfy f(x, arg), reject all those that don't, and close the
 * resulting gaps by contracting the space between all remaining items, thus preserving the relative order
 * of the remaining items. All rejected items are moved to a new vector in the same relative order in which
 * they appeared in the original vector. The filter is applied linearly from first to last item. O(n).
 * @param f Some predicate to filter the vector.
 * @param arg An optional argument passed to the predicate.
 * @return The new axvector containing all rejected items or NULL if OOM, in which case no filtering is done.
 */
axvector *axv_filterSplit(axvector *v, bool (*f)(const void *, void *), void *arg);
/**
 * Let f be a function taking (item in vector, optional argument).
 * Call f(x, arg) on every item x until f returns false or all items of the vector have been exhausted.
 * Items are iterated linearly from first to last.
 * @param f Function to call on items.
 * @param arg An optional argument passed to the function.
 * @return Self.
 */
axvector *axv_foreach(axvector *v, bool (*f)(void *, void *), void *arg);
/**
 * Let f be a function taking (item in vector, optional argument).
 * Call f(x, arg) on every item x until f returns false or all items of the vector have been exhausted.
 * Items are iterated linearly from last to first.
 * @param f Function to call on items.
 * @param arg An optional argument passed to the function.
 * @return Self.
 */
axvector *axv_rforeach(axvector *v, bool (*f)(void *, void *), void *arg);
/**
 * Let f be a function taking (item in vector, optional argument).
 * Call f(x, arg) on every item x in some section until f returns false or all items of the section have
 * been exhausted. Items are iterated linearly from first to last.
 * @param f Function to call on items.
 * @param arg An optional argument passed to the function.
 * @param index1 Beginning of section. May be negative. Inclusive.
 * @param index2 End of section. May be negative. Exclusive.
 * @return Self.
 */
axvector *axv_forSection(axvector *v, bool (*f)(void *, void *), void *arg,
                         int64_t index1, int64_t index2);
/**
 * Check if vector is sorted according to comparator. Items are checked linearly from first to last.
 * @return True if sorted, false if not.
 */
bool axv_isSorted(axvector *v);
/**
 * Sort vector using its comparator.
 * @return Self.
 */
axvector *axv_sort(axvector *v);
/**
 * Sort section of vector using its comparator.
 * @param index1 Beginning of section. May be negative. Inclusive.
 * @param index2 End of section. May be negative. Exclusive.
 * @return Self.
 */
axvector *axv_sortSection(axvector *v, int64_t index1, int64_t index2);
/**
 * Binary search the argument in the vector. Only applicable if vector is sorted. No check is done for this.
 * @param val Value to search using the vector's comparator.
 * @return Index of any item which matches the argument or -1 if no such item is found.
 */
int64_t axv_binarySearch(axvector *v, void *val);
/**
 * Linear search the argument in the vector. Forward search is used.
 * @param val Value to search using the vector's comparator.
 * @return Index of the first item which matches the argument or -1 if no such item is found.
 */
int64_t axv_linearSearch(axvector *v, void *val);
/**
 * Linear search the argument in some section of the vector. Forward search is used.
 * @param val Value to search using the vector's comparator.
 * @param index1 Beginning of section. May be negative. Inclusive.
 * @param index2 End of section. May be negative. Exclusive.
 * @return Index of the first item which matches the argument or -1 if no such item is found.
 */
int64_t axv_linearSearchSection(axvector *v, void *val, int64_t index1, int64_t index2);
/**
 * Set comparator function. Type must match int (*)(const void *, const void *).
 * Refer to the C standard on the definition of a compliant comparator function.
 * @param cmp Comparator or NULL to activate default comparator (compares addresses).
 * @return Self.
 */
axvector *axv_setComparator(axvector *v, int (*cmp)(const void *, const void *));
/**
 * Get comparator function. Type is int (*)(const void *, const void *).
 * @return Comparator.
 */
int (*axv_getComparator(axvector *v))(const void *, const void *);
/**
 * Set destructor function. Type must match void (*)(void *), which matches i.e. the free() function.
 * @param destroy Destructor or NULL to deactivate destructor features.
 * @return Self.
 */
axvector *axv_setDestructor(axvector *v, void (*destroy)(void *));
/**
 * Get destructor function. Type is void (*)(void *).
 * @return Destructor or NULL if not set.
 */
void (*axv_getDestructor(axvector *v))(void *);
/**
 * Store a context in the vector.
 * @param context Context.
 * @return Self.
 */
axvector *axv_setContext(axvector *v, void *context);
/**
 * Get the stored context of this vector.
 * @return Context.
 */
void *axv_getContext(axvector *v);
/**
 * Pointer to first item of this vector. This function is useful when you need raw array access.
 * @return The internal array of this vector.
 */
void **axv_data(axvector *v);
/**
 * Capacity of this vector. The capacity is the maximum number of items that fit without the need of resizing
 * the vector.
 * @return Capacity.
 */
int64_t axv_cap(axvector *v);


#endif //AXVECTOR_AXVECTOR_H
