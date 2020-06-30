/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-30 12:02:18
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-30 12:52:39
 */

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>

struct Stock {
  Stock() = default;
  Stock(const std::string& key) : _key(key) {}
  Stock(const Stock& other) : _key{other._key} {}
  ~Stock() { std::cout << "~Stock()" << std::endl; }

  std::string key() const { return _key; }

  Stock& operator=(const Stock& other) {
    this->_key = other._key;
    return (*this);
  }

  std::string _key;
};

class StockFactory : public std::enable_shared_from_this<StockFactory> {
 public:
  std::shared_ptr<Stock> get(const std::string& key) {
    std::shared_ptr<Stock> pStock;
    std::lock_guard<std::mutex> lock(_mutex);
    std::weak_ptr<Stock>& wkStock = _stocks[key];
    pStock = wkStock.lock();
    if (!pStock) {
      pStock.reset(new Stock(key),
                   std::bind(&StockFactory::weakDeleteCallBack,
                             std::weak_ptr<StockFactory>(shared_from_this()),
                             std::placeholders::_1));
      // pStock.reset(new Stock(key),
      // std::bind(&StockFactory::weakDeleteCallBack2,
      //                                        this, std::placeholders::_1));
      wkStock = pStock;
    }
    return pStock;
  }

 private:
  static void weakDeleteCallBack(const std::weak_ptr<StockFactory>& wkFactory,
                                 Stock* stock) {
    std::shared_ptr<StockFactory> factory(wkFactory.lock());
    if (factory) {
      factory->removeStock(stock);
    }
    delete stock;
  }
  static void weakDeleteCallBack2(StockFactory* factory, Stock* stock) {
    if (factory) {
      //  std::cout << factory << std::endl;
      factory->removeStock(stock);
    }
    delete stock;
  }

  void removeStock(Stock* stock) {
    if (stock) {
      std::lock_guard<std::mutex> lock(_mutex);
      _stocks.erase(stock->key());
    }
  }

 private:
  std::mutex _mutex;
  std::map<std::string, std::weak_ptr<Stock>> _stocks;
};

void testLongLifeFactory() {
  std::shared_ptr<StockFactory> factory = std::make_shared<StockFactory>();
  {
    std::shared_ptr<Stock> stock = factory->get("NYSE:IBM");
    std::shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
    assert(stock == stock2);
    // stock destructs here
  }
  // factory destructs here
}

void testShortLifeFactory() {
  std::shared_ptr<Stock> stock;
  {
    std::shared_ptr<StockFactory> factory = std::make_shared<StockFactory>();
    stock = factory->get("NYSE:IBM");
    std::shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
    assert(stock == stock2);
    // factory destructs here
  }
  // stock destructs here
}

int main() {
  testLongLifeFactory();
  testShortLifeFactory();
  return 0;
}