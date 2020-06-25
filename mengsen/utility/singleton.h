/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-25 18:23:19
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-25 18:33:43
 * @Description: singleton back up
 */

#ifndef __MENGSEN_SINGLETON_H__
#define __MENGSEN_SINGLETON_H__

#include <memory>

namespace mengsen {

namespace {

template <class T, class X, int N>
T& GetInstanceX() {
  static T v;
  return v;
}

template <class T, class X, int N>
std::shared_ptr<T> GetInstancePtr() {
  static std::shared_ptr<T> v(new T);
  return v;
}

}  // namespace

/**
 * @brief: singleto instance back up
 * @details: [template] T type
 * @details: [template] X To create multiple instances of corresponding tags
 * @details: [template] N Create multiple instance indexes for the same Tag
 */
template <class T, class X = void, int N = 0>
class Singleton {
 public:
  /**
   * @brief: get instance for singleton
   * @param: void
   * @return: raw pointer for singleton object
   */
  static T* GetInstance() {
    static T v;
    return &v;
    // return &GetInstanceX<T, X, N>();
  }
};

/**
 * @brief: singleton shared pointer back up
 * @details: [template] T type
 * @details: [template] X To create multiple instances of corresponding tags
 * @details: [template] N Create multiple instance indexes for the same Tag
 */
template <class T, class X = void, int N = 0>
class SingletonPtr {
 public:
  /**
   * @brief: get shared_ptr with singleton
   */
  static std::shared_ptr<T> GetInstance() {
    static std::shared_ptr<T> v = std::make_shared<T>();
    return v;
    // return GetInstancePtr<T, X, N>();
  }
};

}  // namespace mengsen

#endif  // __MENGSEN_SINGLETON_H__
