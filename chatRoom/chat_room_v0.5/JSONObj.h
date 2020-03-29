/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-29 11:27:18
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-29 11:39:53
 */

#ifndef JSONOBJ_H
#define JSONOBJ_H
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <sstream>
#include <string>

using ptree = boost::property_tree::ptree;

inline std::string ptreeToJsonString(const ptree& tree) {
  std::stringstream ss;
  boost::property_tree::write_json(ss, tree, false);
  return ss.str();
}

#endif  // JSONOBJ_H