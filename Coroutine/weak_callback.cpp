/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-09 17:36:56
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-09 18:47:01
 * @Description: 弱回调
 */

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
  ~Stock() = default;

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

  std::shared_ptr<Stock> get() const
      noexcept = delete;  // const value can not invoke

  /**
   * 传入一个key，map会生成一个指针 这个指针是空指针
   * 如果不返回引用的话绑定的对象其实是临时对象
   * 相当于返回一个空指针回来，而不是map中的那个空指针
   * 后续给空指针构造的时候就相当于未加入map
   * 但是由于 shared_ptr 的原因不会马上析构，但是引用计数会变成1
   */
  std::shared_ptr<Stock>& get(
      const std::string& key) noexcept  //通过key获取对应的value
  {
    std::shared_ptr<Stock> temp_stock{nullptr};

    std::lock_guard<std::mutex> lock_guard(this->mutex);  // lock
    std::weak_ptr<Stock>& temp_weak =
        (this->stocks)[key];  //查找map中是否含有该key对应的value.

    temp_stock = temp_weak.lock();

    // != true
    if (!temp_stock) {
      //如果temp_stock为nullptr那么插入数据到该value中.
      // 第二次不会走到这里
      temp_stock.reset(
          new Stock{key},
          std::bind(
              &StockFactory::
                  weak_deleter_call_back,  //这里我们设置了自定义的deleter,保证当该stock没有引用对象的时候能够从map中被移除掉.
              std::weak_ptr<StockFactory>{this->shared_from_this()},
              std::placeholders::_1));

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
      temp_factory->remove_stcock(stock);
    }

    delete stock;
    stock = nullptr;
  }

  void remove_stock(Stock* stock) const noexcept = delete;
  void remove_stcock(Stock* stock) noexcept {
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
  {
    std::shared_ptr<Stock> temp_stock1{shared_factory->get("shihua")};
    std::cout << temp_stock1.use_count() << "-------------" << std::endl;
    std::shared_ptr<Stock> temp_stock2{shared_factory->get("shihua")};
    std::cout << temp_stock2.use_count() << "-------------" << std::endl;
  }

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
