#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>

template <typename T>
using Weak = std::weak_ptr<T>;
template <typename T>
using Shared = std::shared_ptr<T>;
template <typename T>
using Unique = std::unique_ptr<T>;
template <typename T>
using List = std::vector<T>;
template <typename T, typename U>
using Map = std::map<T, U>;
template <typename T, typename U>
using UMap = std::unordered_map<T, U>;
using String = std::string;

