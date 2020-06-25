/*
 * @Description: config module
 * @Author: Mengsen.Wang
 * @Date: 2020-06-19 15:34:53
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-23 22:52:46
 */

#ifndef __MENGSEN_CONFIG_H__
#define __MENGSEN_CONFIG_H__

#include <yaml-cpp/yaml.h>  // for YAML

#include <boost/lexical_cast.hpp>  // for LexicalCast and include base type of STL
#include <iostream>                // for stringstream
#include <memory>                  // for smart pointer
#include <shared_mutex>            // for read-write lock
#include <string>
#include <unordered_map>  // for LexicalCast
#include <unordered_set>  // for LexicalCast

namespace {

/**
 * @brief: Type conversion template class
 * @param: from typename F to typename T
 */
template <class F, class T>
class LexicalCast {
 public:
  /**
   * @brief: convert from F to T
   * @param: [in] const F& val
   * @return: T
   * @exception: throw bad_lexical_cast
   */
  T operator()(const F& val) { return boost::lexical_cast<T>(val); }
};

/**
 * @brief: Partial specialization from YAML string to vector<T>
 */
template <class T>
class LexicalCast<std::string, std::vector<T>> {
 public:
  /**
   * @brief: operator()
   * @param: [in] const std::string & val
   * @return: std::vector<T>
   */
  std::vector<T> operator()(const std::string& val) {
    YAML::Node node = YAML::Load(val);
    typename std::vector<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      // clear ss
      ss.str("");
      // set ss string from node[i]
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>()(ss.str));
    }
    return vec;
  }
};

/**
 * @brief: Partial specialization from vector<F> to YAML string
 */
template <class F>
class LexicalCast<std::vector<F>, std::string> {
 public:
  /**
   * @brief: operator()
   * @param: const std::vector<F>& vec
   * @return: std::string
   */
  std::string operator()(const std::vector<F>& vec) {
    // init node to sequence type
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : vec) {
      node.push_back(YAML::Load(LexicalCast<F, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief: Partial specialization from YAML string to std::list<T>
 */
template <class T>
class LexicalCast<std::string, std::list<T>> {
  /**
   * @brief: operator()
   * @param: const std::string& val
   * @return: std::list<T>
   */
  std::list<T> operator()(const std::string& val) {
    YAML::Node node = YAML::Load(val);
    typename std::list<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief: Partial specialization from std::list<F> to YAML std::string
 */
template <class F>
class LexicalCast<std::list<F>, std::string> {
 public:
  /**
   * @brief: operator()
   * @param: const std::list<F>& val
   * @return: std::string
   */
  std::string operator()(const std::list<F>& val) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : val) {
      node.push_back(YAML::Load(LexicalCast<F, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief: Partial specialization from YAML std::string to std::set<T>
 */
template <class T>
class LexicalCast<std::string, std::set<T>> {
 public:
  /**
   * @brief: operator()
   * @param: const std::string val
   * @return: std::set<T>
   */
  std::set<T> operator()(const std::string& val) {
    YAML::Node node = YAML::Load(val);
    typename std::set<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief: Partial specialization from std::set<T> to YAML std::string
 */
template <class F>
class LexicalCast<std::set<F>, std::string> {
 public:
  /**
   * @brief: operator()
   * @param: const std::set<> val
   * @return: std::set<T>
   */
  std::string operator()(const std::set<F>& val) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : val) {
      node.push_back(YAML::Load(LexicalCast<F, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief: From YAML std::string to std::unordered_set<T>
 */
template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
 public:
  /**
   * @brief: operator()
   * @param: const std::string val
   * @return: const std::unordered_set<T> val
   */
  std::unordered_set<T> operator()(const std::string& val) {
    YAML::Node node = YAML::Load(val);
    typename std::unordered_set<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief: From std::unordered_set<T> to YAML std::string
 */
template <class F>
class LexicalCast<std::unordered_set<F>, std::string> {
 public:
  /**
   * @brief: operator()
   * @param: const std::unordered_set<T> val
   * @return: std::string
   */
  std::string operator()(const std::unordered_set<F>& val) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : val) {
      node.push_back(YAML::Load(LexicalCast<F, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief: From YAML std::string to std::map<std::string, T>
 */
template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
 public:
  /**
   * @brief: operator()
   * @param: std::string val
   * @return: const std::map<std::string, T>
   */
  std::map<std::string, T> operator()(const std::string& val) {
    YAML::Node node = YAML::Load(val);
    typename std::map<std::string, T> vec;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      vec.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>()(ss.str())));
    }
    return vec;
  }
};

/**
 * @brief: From std::map<std::string, F> to YAML std::string
 */
template <class F>
class LexicalCast<std::map<std::string, F>, std::string> {
 public:
  /**
   * @brief: operator()
   * @param: const std::map<std::string, T> val
   * @return: std::string
   */
  std::string operator()(const std::map<std::string, F>& val) {
    YAML::Node node(YAML::NodeType::Map);
    for (auto& i : val) {
      node[i.first] = YAML::Load(LexicalCast<F, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief: From YAML std::string to std::unordered_map<std::string, T>
 */
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
 public:
  /**
   * @brief: operator()
   * @param: std::string val
   * @return: const std::unordered_map<std::string, T>
   */
  std::unordered_map<std::string, T> operator()(const std::string& val) {
    YAML::Node node = YAML::Load(val);
    typename std::unordered_map<std::string, T> vec;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      vec.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>()(ss.str())));
    }
    return vec;
  }
};

/**
 * @brief: From std::unordered_map<std::string, F> to YAML std::string
 */
template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
 public:
  /**
   * @brief: operator()
   * @param: const std::unordered_map<std::string, T> val
   * @return: std::string
   */
  std::string operator()(const std::unordered_map<std::string, T>& val) {
    YAML::Node node(YAML::NodeType::Map);
    for (auto& i : val) {
      // yaml first->second
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

}  // namespace

namespace mengsen_config {
/**
 * @brief: base class of ConfigVar
 */
class ConfigVarBase {
 public:
  /**
   * @brief: shared_ptr of ConfigVarBase
   */
  typedef std::shared_ptr<ConfigVarBase> ptr;

  /**
   * @brief: ConfigVarBase constructor
   * @param: [in] std::string name (configuration name)
   * @param: [in] std::string description (configuration description default
   * "")
   */
  ConfigVarBase(const std::string& name, const std::string& description = "");

  /**
   * @brief: destructor
   */
  virtual ~ConfigVarBase() {}

  /**
   * @brief: return configuration name
   */
  const std::string& getName() const { return _name; }

  /**
   * @brief: return configuration description
   */
  const std::string& getDescription() const { return _description; }

  /**
   * @brief: convert to std::string
   */
  virtual std::string toString() = 0;

  /**
   * @brief: initializes a value from a string
   * @param: [in] std::string val (initialize value)
   */
  virtual bool fromString(const std::string& val) = 0;

  /**
   * @brief: return configuration paramter type
   */
  virtual std::string getTypeName() const = 0;

  // for subclass visibility
 protected:
  // name of configuration parameters
  std::string _name;
  // description of confignation parameters
  std::string _description;
};

/**
 * @brief: subclass of ConfigVarBase
 * @description:
 * typename T  parameter type
 * class FromStr convert type std::string to T
 * class ToStr convert type T to std::string
 */
template <typename T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVar> ptr;
  typedef std::function<void(const T& old_value, const T& new_value)>
      change_func;

  /**
   * @brief: constructor with ConfigVar
   * @param: [in] std::string& name (Config name)
   * @param: [in] const T& default_value (Config default_value)
   * @param: [in] const std::string& description (Config description)
   */
  ConfigVar(const std::string& name, const T& default_value,
            const std::string& description = "");

  ~ConfigVar() = default;

  /**
   * @brief: convert T to YAML string
   */
  std::string toString() override;

  /**
   * @brief: convert YAML string to T _value
   */
  bool fromString(const std::string& val) override;

  /**
   * @brief: get _value
   */
  const T getValue() const;

  /**
   * @brief: set _value
   * If the value of the parameter changes,
   * the corresponding registered callback function is notified
   */
  void setValue(const T& val);

  /**
   * @brief: get value typeinfo
   */
  std::string getTypeName() const override;

  /**
   * @brief: add call back function
   * @return: uint64_t key (the unique id fo call back function)
   */
  uint64_t addListener(change_func fun);

  /**
   * @brief: delete call back function
   * @param: [in] uint64_t key (the unique id fo call back function)
   * @return: void
   */
  void deleteListener(uint64_t key);

  /**
   * @brief: get call back function
   * @param: [in] uint64_t key (the unique id fo call back function)
   * @return: change_func
   */
  change_func getListener(uint64_t key);

  /**
   * @brief: clear all of call back function
   * @param: void
   * @return: void
   */
  void clearListener();

 private:
  std::shared_mutex _mutex;
  T _value;
  std::map<uint64_t, change_func> _func;
};

/**
 * @brief: management configVar class
 */
class Config {
 public:
  typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

  /**
   * @brief: get/create configuration
   * @param: [in] const std::string& name (config name)
   * @param: [in] const T& defaule_value (config default value)
   * @param: [in] const std::string& description (config description)
   * @return: corresponding ConfigVar pointer or nullptr
   * @exception: throw std::invalid_argument
   */
  template <typename T>
  static typename ConfigVar<T>::ptr Lookup(const std::string& name,
                                           const T& default_value,
                                           const std::string& description = "");

  /**
   * @brief: get ConfigVar pointer
   * @param: [in] const std::string& name (config name)
   * @return: ConfigVar pointer
   */
  template <typename T>
  static typename ConfigVar<T>::ptr Lookup(const std::string& name);

  /**
   * @brief: initialize this part use YAML::Node
   * @param: [in] const YAML::Node& node (use node init)
   * @return: void
   */
  static void LoadFromYaml(const YAML::Node& root);

  /**
   * @brief: load configuration file in path
   * @param: [in] const std::string& path (file path)
   * @param: [in] bool force = false
   * @return: void
   */
  static void LoadFromConfDir(const std::string& path, bool force = false);

  /**
   * @brief: find ConfigVar with name
   * @param: [in] const std::string& name (config name)
   * @return: ConfigVarBase pointer
   */
  static ConfigVarBase::ptr LookupBase(const std::string& name);

  /**
   * @brief: Walk through all configuration items in the configuration module
   * @param: [in] std::function<void(ConfigVarBase::ptr)> func
   * @return: void
   */
  static void Visit(std::function<void(ConfigVarBase::ptr)> func);

 private:
  static std::shared_mutex _mutex;
  static ConfigVarMap _data;
};

}  // namespace mengsen_config

#endif  // __MENGSEN_CONFIG_H__