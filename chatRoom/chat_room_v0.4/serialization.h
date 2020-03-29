/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-29 08:51:12
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-29 11:06:15
 */

#ifndef SERI_H
#define SERI_H
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <string>
class SBindName {
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& m_bindName;
  }
  std::string m_bindName;

 public:
  SBindName(std::string name) : m_bindName(std::move(name)) {}
  SBindName() {}
  const std::string& bindName() const { return m_bindName; }
};

class SChatInfo {
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& m_chatInfomation;
  }
  std::string m_chatInfomation;

 public:
  SChatInfo(std::string info) : m_chatInfomation(std::move(info)) {}
  SChatInfo() {}
  const std::string& chatInfomation() const { return m_chatInfomation; }
};

class SRoomInfo {
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& m_bind;
    ar& m_chat;
  }
  SBindName m_bind;
  SChatInfo m_chat;

 public:
  SRoomInfo(std::string name, std::string info)
      : m_bind(std::move(name)), m_chat(std::move(info)) {}
  SRoomInfo() {}
  const std::string& name() const { return m_bind.bindName(); }
  const std::string& information() const { return m_chat.chatInfomation(); }
};
#endif  // SERI_H