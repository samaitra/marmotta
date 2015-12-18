/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// A collection of iterators used by different components.
//

#ifndef MARMOTTA_ITERATOR_H
#define MARMOTTA_ITERATOR_H

namespace marmotta {
namespace util {

/**
 * A common iterator class for iterators binding resources.
 */
template<typename T>
class CloseableIterator {
 public:

    /**
     * Close the iterator, freeing any wrapped resources
     */
    virtual ~CloseableIterator() {}

    /**
     * Increment iterator to next element.
    */
    virtual CloseableIterator<T>& operator++() = 0;

    /**
     * Dereference iterator, returning a reference to the current element.
     */
    virtual T& operator*() = 0;

    /**
     * Dereference iterator, returning a pointer to the current element.
     */
    virtual T* operator->() = 0;

    /**
     * Return true in case the iterator has more elements.
     */
    virtual bool hasNext() = 0;

};

/**
 * An empty iterator.
 */
template<typename T>
class EmptyIterator : public CloseableIterator<T> {
 public:
    EmptyIterator() { }

    CloseableIterator<T> &operator++() override {
        return *this;
    }

    T &operator*() override {
        throw std::out_of_range("No more elements");
    };

    T *operator->() override {
        throw std::out_of_range("No more elements");
    };

    bool hasNext() override {
        return false;
    };
};



/**
 * An iterator wrapping a single element.
 */
template<typename T>
class SingletonIterator : public CloseableIterator<T> {
 public:
    SingletonIterator(T& value) : value(value), incremented(false) { }

    CloseableIterator<T> &operator++() override {
        incremented = true;
        return *this;
    };

    T &operator*() override {
        if (!incremented)
            return value;
        else
            throw std::out_of_range("No more elements");
    };

    T *operator->() override {
        if (!incremented)
            return &value;
        else
            throw std::out_of_range("No more elements");
    };

    bool hasNext() override {
        return !incremented;
    };

 private:
    T value;
    bool incremented;

};


template<typename T>
class FilteringIterator : public CloseableIterator<T> {
 public:
    using PredicateFn = std::function<bool(const T&)>;

    /**
     * Create a filtering iterator using the given predicate as filter.
     * The predicate function should return true for accepted values.
     * Takes ownership of the iterator passed as argument.
     */
    FilteringIterator(CloseableIterator<T>* it,  PredicateFn pred)
            : it(it), pred(pred), nextExists(false) {
        // run findNext twice so both current and next are set
        findNext();
        findNext();
    }

    /**
     * Increment iterator to next element.
    */
    CloseableIterator<T>& operator++() override {
        findNext();
        return *this;
    };

    /**
     * Dereference iterator, returning a reference to the current element.
     */
    T& operator*() override {
        return current;
    };

    /**
     * Dereference iterator, returning a pointer to the current element.
     */
    T* operator->() override {
        return &current;
    };

    /**
     * Return true in case the iterator has more elements.
     */
    bool hasNext() override {
        return nextExists;
    };


 private:
    std::unique_ptr<CloseableIterator<T>> it;
    PredicateFn pred;

    T current;
    T next;

    bool nextExists;

    void findNext() {
        current = next;
        nextExists = false;

        while (it->hasNext()) {
            if (pred(**it)) {
                next = **it;
                nextExists = true;
                break;
            }
        }
    }
};


}
}


#endif //MARMOTTA_ITERATOR_H
