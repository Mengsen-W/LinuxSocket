/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-22 20:58:21
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-23 22:09:15
 */

#include "config.h"

#include "../log/log.h"
#include "../util/util.h"

namespace mengsen_config {

template <typename T, class FromStr, class ToStr>
ConfigVar<T, FromStr, ToStr>::ConfigVar(const std::string& name,
                                        const T& default_value,
                                        const std::string& description = "")
    : ConfigVarBase(name, description), _value(default_value) {}

template <typename T, class FromStr, class ToStr>
std::string ConfigVar<T, FromStr, ToStr>::toString() {
  try {
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return ToStr()(_value);
  } catch (std::exception& e) {
    LOG_ERROR << "ConfigVar::toString exception " << e.what()
              << " convert: " << TypeToName<T>() << " to string"
              << " name=" << _name;
  }
  return "";
}

template <typename T, class FromStr, class ToStr>
bool ConfigVar<T, FromStr, ToStr>::fromString(const std::string& val) {
  try {
    setValue(FromStr()(val));
  } catch (std::exception& e) {
    LOG_ERROR << "ConfigVar::fromString exception " << e.what()
              << " convert: string to " << TypeToName<T>() << " name=" << _name
              << " - " << val;
  }
  return false;
}

template <typename T, class FromStr, class ToStr>
const T ConfigVar<T, FromStr, ToStr>::getValue() const {
  std::shared_lock<std::shared_mutex> lock(_mutex);
  return _value;
}

template <typename T, class FromStr, class ToStr>
void ConfigVar<T, FromStr, ToStr>::setValue(const T& val) {
  {
    std::shared_lock<std::shared_mutex> lock(_mutex);
    if (val == _value) {
      return;
    }
    for (auto& i : _func) {
      i.second(_value, val);
    }
  }
  std::unique_lock<std::shared_mutex> lock(_mutex);
  _value = val;
}

template <typename T, class FromStr, class ToStr>
std::string ConfigVar<T, FromStr, ToStr>::getTypeName() const {
  return TypeToName<T>();
}

template <typename T, class FromStr, class ToStr>
uint64_t ConfigVar<T, FromStr, ToStr>::addListener(change_func fun) {
  static uint64_t func_id = 0;
  std::unique_lock<std::shared_mutex> lock(_mutex);
  ++func_id;
  _func[func_id] = fun;
  return func_id;
}

template <typename T, class FromStr, class ToStr>
void ConfigVar<T, FromStr, ToStr>::deleteListener(uint64_t key) {
  std::unique_lock<std::shared_mutex> lock(_mutex);
  _func.erase(key);
  return;
}

template <typename T, class FromStr, class ToStr>
ConfigVar<T, FromStr, ToStr>::change_func
ConfigVar<T, FromStr, ToStr>::getListener(uint64_t key) {
  std::shared_lock<std::shared_mutex> lock(_mutex);
  std::map<uint64_t, change_func>::iterator it = _func.find(key);
  return it == _func.end() ? nullptr : it->second;
}

template <typename T, class FromStr, class ToStr>
void ConfigVar<T, FromStr, ToStr>::clearListener() {
  std::unique_lock<std::shared_lock> lock(_mutex);
  _func.clear();
}

template <typename T>
typename ConfigVar<T>::ptr Config::Lookup(const std::string& name,
                                          const T& default_value,
                                          const std::string& description = "") {
}

}  // namespace mengsen_config
