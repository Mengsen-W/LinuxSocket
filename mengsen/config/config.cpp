/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-22 20:58:21
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-22 22:00:01
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
    std::lock_shared<std::shared_mutex> lock(_mutex);
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
const T ConfigVar<T, FromStr, ToStr>::getValue() const {}

template <typename T, class FromStr, class ToStr>
void ConfigVar<T, FromStr, ToStr>::setValue(const T& val) {
  {
    std::lock_shared<std::shared_mutex> lock(_mutex);
    if (v == _val) {
      return;
    }
    for (auto& i : _func) {
      i.second(_val, v);
    }
  }
  std::unique_lock<std::shared_mutex> lock(_mutex);
  _val = val;
}
}  // namespace mengsen_config
