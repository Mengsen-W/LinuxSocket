/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-09 18:50:22
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-30 12:15:21
 *
 */

#include <unistd.h>

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

struct Stock {
  std::string name{"none"};
  int code{0};

  Stock() = default;
  ~Stock() { std::cout << "~Stock()" << std::endl; }

  Stock(const std::string& the_name) : name{the_name} {}

  Stock(const Stock& other) : name{other.name} {}

  Stock& operator=(const Stock& other) {
    this->name = other.name;
    this->code = other.code;
    return (*this);
  }
};

class StockFactory : public std::enable_shared_from_this<StockFactory> {
 public:
  StockFactory() = default;

  ~StockFactory() { std::cout << "destroy the factory!" << std::endl; }

  StockFactory(const StockFactory& other) = delete;             //!
  StockFactory& operator=(const StockFactory& other) = delete;  //!

  std::shared_ptr<Stock> get() const noexcept =
      delete;  // const value can not invoke

  /**
   * map 中使用 weak_ptr 的作用是防止重复引用
   * 这样socket永远不会被销毁
   * 只有在整个 StockFactory 被销毁时才会被销毁
   * -----------------------
   * 传入一个key，map会生成一个指针 这个指针是空指针
   * 如果不返回引用的话绑定的对象其实是临时对象
   * 相当于返回一个空指针回来，而不是map中的那个空指针
   * 后续给空指针构造的时候就相当于未加入map
   * 但是由于 shared_ptr 的原因不会马上析构，但是引用计数会变成1
   * -----------------------
   * 这里不能返回引用这样会使得 shared_ptr 里面所有东西在返回时直接被析构
   * -----------------------
   * reset在这里的作用是绑定一个析构函数，而不是调用默认的析构函数
   * 如果没有一个单独的析构函数可能导致的问题是
   * map里面一直存在这一个 key-value 只不过 value 的值是 nullptr
   * -----------------------
   * 为了定制析构函数必须要使用 reset 但是这里 reset 又必须拿到 this 指针
   * 否则在析构的时候 StockFactory map 不会被删除
   * 为了防止多线程下的线程安全所以必须拿到 this 的智能指针
   * 这样就导致了另一个问题 StockFactory 的生命周期被延长了
   * -----------------------
   * 所以中间要判断factoy是否存在
   * 因为 shared_ptr 其实指向的是一个 stock*
   * 两个线程其中一个析构了 StockFactory 另一个现在在析构的某个时刻
   * 这样两个资源谁先挂掉都不会影响程序的正常执行
   * 有时候我们需要 “如果对象还活着我们才调用成员函数”
   */
  std::shared_ptr<Stock> get(
      const std::string& key) noexcept  //通过key获取对应的value
  {
    std::shared_ptr<Stock> temp_stock{nullptr};

    // 不会同时有多个线程到下面
    std::lock_guard<std::mutex> lock_guard(this->mutex);  // lock
    std::weak_ptr<Stock>& temp_weak =
        (this->stocks)[key];  //查找map中是否含有该key对应的value.

    temp_stock = temp_weak.lock();  // 提升

    // != true
    if (!temp_stock)  //如果temp_stock为nullptr那么插入数据到该value中.
    {
      std::cout << "no key" << std::endl;
      // 用 temp_stock 构造
      temp_stock.reset(
          new Stock{key},
          std::bind(
              &StockFactory::
                  weak_deleter_call_back,  //这里我们设置了自定义的deleter,保证当该stock没有引用对象的时候能够从map中被移除掉.
              std::weak_ptr<StockFactory>{this->shared_from_this()},
              std::placeholders::_1));

      // weak_ptr operator= means copy but don't add reference number
      temp_weak = temp_stock;
    }

    return temp_stock;
  }

 private:
  static void weak_deleter_call_back(
      const std::weak_ptr<StockFactory>& weak_ptr, Stock* stock) noexcept {
    std::shared_ptr<StockFactory> temp_factory{weak_ptr.lock()};
    if (temp_factory)  // == true
    {
      std::cout << "destroy: " << stock->name << std::endl;
      temp_factory->remove_stock(stock);
    }

    delete stock;
    stock = nullptr;
  }

  void remove_stock(Stock* stock) noexcept {
    if (stock != nullptr) {
      std::lock_guard<std::mutex> lock_guard{this->mutex};
      (this->stocks).erase(stock->name);
    }
  }

  mutable std::mutex mutex{};
  std::map<std::string, std::weak_ptr<Stock>> stocks{};
};

int main() {
  // case 1:
  std::shared_ptr<StockFactory> shared_factory{
      std::make_shared<StockFactory>()};
  std::shared_ptr<Stock> temp_stock2{shared_factory->get("shihua")};
  {
    std::shared_ptr<Stock> temp_stock1{shared_factory->get("shihua")};
    std::cout << temp_stock1.use_count() << "-------------" << std::endl;
  }
  sleep(5);
  std::cout << temp_stock2.use_count() << "-------------" << std::endl;

  std::cout << std::endl;

  // case 2:
  // std::shared_ptr<Stock> shared_stock{nullptr};
  // {
  //   std::shared_ptr<StockFactory> temp_factory{
  //       std::make_shared<StockFactory>()};
  //   shared_stock = temp_factory->get("shihua");
  //   std::shared_ptr<Stock> shared_stock2{temp_factory->get("shihua")};
  //   assert(shared_stock == shared_stock2);
  // }

  return 0;
}
