/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-22 20:58:21
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-25 19:23:20
 */

#include "config.h"

#include <sys/stat.h>
#include <unistd.h>

#include "../environment/env.h"
#include "../log/log.h"
#include "../utility/utility.h"

namespace mengsen_config {

ConfigVarBase::ConfigVarBase(const std::string& name,
                             const std::string& description)
    : _name(name), _description(description) {
  std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
}

template <typename T, class FromStr, class ToStr>
ConfigVar<T, FromStr, ToStr>::ConfigVar(const std::string& name,
                                        const T& default_value,
                                        const std::string& description)
    : ConfigVarBase(name, description), _value(default_value){};

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
typename ConfigVar<T, FromStr, ToStr>::change_func
ConfigVar<T, FromStr, ToStr>::getListener(uint64_t key) {
  std::shared_lock<std::shared_mutex> lock(_mutex);
  typename std::map<uint64_t, change_func>::iterator it = _func.find(key);
  return it == _func.end() ? nullptr : it->second;
}

template <typename T, class FromStr, class ToStr>
void ConfigVar<T, FromStr, ToStr>::clearListener() {
  std::unique_lock<std::shared_mutex> lock(_mutex);
  _func.clear();
}

template <typename T>
typename ConfigVar<T>::ptr Config::Lookup(const std::string& name,
                                          const T& default_value,
                                          const std::string& description) {
  std::unique_lock<std::shared_mutex> lock(_mutex);
  auto it = _data.find(name);

  if (it != _data.end()) {
    auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    if (tmp) {
      LOG_INFO << "Lookup name=" << name << " exists";
      return tmp;
    } else {
      LOG_ERROR << "Lookup name=" << name << " exists but type not "
                << TypeToName<T>() << " real_type=" << it->second->getTypeName()
                << " " << it->second->toString();
      return nullptr;
    }
  }

  if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") !=
      std::string::npos) {
    LOG_ERROR << "Lookup name invalid " << name;
    throw std::invalid_argument(name);
  }

  typename ConfigVar<T>::ptr v(
      new ConfigVar<T>(name, default_value, description));
  _data[name] = v;
  return v;
}

template <typename T>
typename ConfigVar<T>::ptr Config::Lookup(const std::string& name) {
  std::shared_lock<std::shared_mutex> lock(_mutex);
  auto it = _data.find(name);
  if (it == _data.end()) {
    return nullptr;
  }
  return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
}

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
  std::shared_lock<std::shared_mutex> lock(_mutex);
  auto it = _data.find(name);
  return it == _data.end() ? nullptr : it->second;
}

//"A.B", 10
// A:
//  B: 10
//  C: str

static void ListAllMember(
    const std::string& prefix, const YAML::Node& node,
    std::list<std::pair<std::string, const YAML::Node> >& output) {
  if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") !=
      std::string::npos) {
    LOG_ERROR << "Config invalid name: " << prefix << " : " << node;
    return;
  }
  output.push_back(std::make_pair(prefix, node));
  if (node.IsMap()) {
    for (auto it = node.begin(); it != node.end(); ++it) {
      ListAllMember(prefix.empty() ? it->first.Scalar()
                                   : prefix + "." + it->first.Scalar(),
                    it->second, output);
    }
  }
}

void Config::LoadFromYaml(const YAML::Node& root) {
  std::list<std::pair<std::string, const YAML::Node> > all_nodes;
  ListAllMember("", root, all_nodes);

  for (auto& i : all_nodes) {
    std::string key = i.first;
    if (key.empty()) {
      continue;
    }

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    ConfigVarBase::ptr var = LookupBase(key);

    if (var) {
      if (i.second.IsScalar()) {
        var->fromString(i.second.Scalar());
      } else {
        std::stringstream ss;
        ss << i.second;
        var->fromString(ss.str());
      }
    }
  }
}

static std::map<std::string, uint64_t> s_file2modifytime;
static std::mutex m_mutex;

void Config::LoadFromConfDir(const std::string& path, bool force) {
  std::string absoult_path =
      mengsen::EnvMgr::GetInstance()->getAbsolutePath(path);
  std::vector<std::string> files;
  FSUtil::ListAllFile(files, absoult_path, ".yml");
  for (auto& i : files) {
    {
      struct stat st;
      lstat(i.c_str(), &st);
      std::lock_guard<std::mutex> lock(m_mutex);
      if (!force && s_file2modifytime[i] == (uint64_t)st.st_mtime) {
        continue;
      }
      s_file2modifytime[i] = st.st_mtime;
    }
    try {
      YAML::Node root = YAML::LoadFile(i);
      LoadFromYaml(root);
      LOG_INFO << "LoadConfFile file=" << i << " ok";
    } catch (...) {
      LOG_ERROR << "LoadConfFile file=" << i << " failed";
    }
  }
}

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
  std::shared_lock<std::shared_mutex> lock(_mutex);
  ConfigVarMap& m = _data;
  for (auto it = m.begin(); it != m.end(); ++it) {
    cb(it->second);
  }
}

}  // namespace mengsen_config
