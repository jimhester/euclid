#pragma once

#include <vector>
#include <string>
#include <typeinfo>
#include <algorithm>
#include <cpp11/logicals.hpp>
#include <cpp11/matrix.hpp>
#include <cpp11/strings.hpp>
#include <cpp11/r_string.hpp>
#include <cpp11/list.hpp>
#include <cpp11/list_of.hpp>
#include <cpp11/external_pointer.hpp>
#include <cpp11/integers.hpp>

#include <sstream>
#include <iomanip>

enum Primitive {
  VIRTUAL,
  CIRCLE,
  DIRECTION,
  ISOCUBE,
  ISORECT,
  LINE,
  PLANE,
  POINT,
  RAY,
  SEGMENT,
  SPHERE,
  TETRAHEDRON,
  TRIANGLE,
  VECTOR,
  WPOINT
};

class geometry_vector_base {
public:
  const Primitive geometry_type = VIRTUAL;

  geometry_vector_base() {}
  virtual ~geometry_vector_base() = default;

  // Conversion
  virtual cpp11::writable::doubles_matrix as_numeric() const = 0;
  virtual cpp11::writable::strings format() const = 0;
  virtual std::vector<double> get_row(size_t i, size_t j) const = 0;

  // Equality
  virtual cpp11::writable::logicals operator==(const geometry_vector_base& other) const = 0;
  virtual cpp11::writable::logicals operator!=(const geometry_vector_base& other) const = 0;

  // Dimensions
  virtual size_t size() const = 0;
  virtual size_t dimensions() const = 0;
  virtual cpp11::writable::strings dim_names() const = 0;
  virtual size_t cardinality(size_t i) const = 0;
  virtual size_t long_length() const = 0;

  // Subsetting etc
  virtual cpp11::external_pointer<geometry_vector_base> subset(cpp11::integers index) const = 0;
  virtual cpp11::external_pointer<geometry_vector_base> copy() const = 0;
  virtual cpp11::external_pointer<geometry_vector_base> assign(cpp11::integers index, const geometry_vector_base& value) const = 0;
  virtual cpp11::external_pointer<geometry_vector_base> combine(cpp11::list_of< cpp11::external_pointer<geometry_vector_base> > extra) const = 0;

  // Self-similarity
  virtual cpp11::external_pointer<geometry_vector_base> unique() const = 0;
  virtual cpp11::writable::logicals duplicated() const = 0;
  virtual int any_duplicated() const = 0;
  virtual cpp11::writable::integers match(const geometry_vector_base& table) const = 0;
  virtual cpp11::writable::logicals is_na() const = 0;
  virtual bool any_na() const = 0;

  // Predicates
  virtual cpp11::writable::logicals is_degenerate() const = 0;
};
typedef cpp11::external_pointer<geometry_vector_base> geometry_vector_base_p;

template <typename T, size_t dim>
class geometry_vector : public geometry_vector_base {
protected:

  std::vector<T> _storage;

public:
  geometry_vector() {}
  // Construct without element copy - BEWARE!
  geometry_vector(std::vector<T> content) {
    _storage.swap(content);
  }
  geometry_vector(const geometry_vector& copy) : _storage(copy._storage) {}
  geometry_vector& operator=(const geometry_vector& copy) const {
    _storage.clear();
    _storage.assign(_storage.end(), copy._storage.begin(), copy._storage.end());
    return *this;
  }
  ~geometry_vector() = default;
  const std::vector<T>& get_storage() const { return _storage; }

  // Conversion
  cpp11::writable::doubles_matrix as_numeric() const {
    cpp11::writable::strings colnames = dim_names();
    size_t ncols = colnames.size();
    cpp11::writable::doubles_matrix result(long_length(), ncols);

    size_t ii = 0;
    for (size_t i = 0; i < size(); ++i) {
      bool is_na = !_storage[i];
      for (size_t j = 0; j < cardinality(i); ++j) {
        std::vector<double> row = get_row(i, j);
        for (size_t k = 0; k < ncols; ++k) {
          result(ii, k) = is_na ? R_NaReal : row[k];
        }
        ++ii;
      }
    }

    result.attr("dimnames") = cpp11::writable::list({R_NilValue, colnames});
    return result;
  }
  cpp11::writable::strings format() const {
    cpp11::writable::strings result(size());
    cpp11::writable::strings dimnames = dim_names();
    size_t ndims = dimnames.size();

    for (size_t i = 0; i < size(); ++i) {
      if (!_storage[i]) {
        result[i] = "<NA>";
        continue;
      }
      std::ostringstream f;
      f << std::setprecision(3);
      size_t car = cardinality(i);
      if (car > 1) {
        f << "<";
      }
      for (size_t j = 0; j < car; ++j) {
        if (j != 0) {
          f << ", ";
        }
        f << "<";
        std::vector<double> row = get_row(i, j);
        for (size_t k = 0; k < ndims; ++k) {
          if (k != 0) {
            f << ", ";
          }
          const std::string name = cpp11::r_string(dimnames[k]);
          f << name << ":" << row[k];
        }
        f << ">";
      }
      if (car > 1) {
        f << ">";
      }

      result[i] = f.str();
    }

    return result;
  }

  // Equality
  cpp11::writable::logicals operator==(const geometry_vector_base& other) const {
    size_t output_length = std::max(size(), other.size());

    cpp11::writable::logicals result(output_length);

    if (typeid(*this) != typeid(other)) {
      for (size_t i = 0; i < size(); ++i) {
        result[i] = (Rboolean) false;
        return result;
      }
    }

    const geometry_vector<T, dim>* other_recast = dynamic_cast< const geometry_vector<T, dim>* >(&other);

    for (size_t i = 0; i < output_length; ++i) {
      if (!_storage[i % size()] || !(*other_recast)[i % other_recast->size()]) {
        result[i] = NA_LOGICAL;
        continue;
      }
      result[i] = (Rboolean) (_storage[i % size()] == (*other_recast)[i % other_recast->size()]);
    }

    return result;
  }
  cpp11::writable::logicals operator!=(const geometry_vector_base& other) const {
    size_t output_length = std::max(size(), other.size());

    cpp11::writable::logicals result(output_length);

    if (typeid(*this) != typeid(other)) {
      for (size_t i = 0; i < size(); ++i) {
        result[i] = (Rboolean) false;
        return result;
      }
    }

    const geometry_vector<T, dim>* other_recast = dynamic_cast< const geometry_vector<T, dim>* >(&other);

    for (size_t i = 0; i < output_length; ++i) {
      if (!_storage[i % size()] || !(*other_recast)[i % other_recast->size()]) {
        result[i] = NA_LOGICAL;
        continue;
      }
      result[i] = (Rboolean) (_storage[i % size()] != (*other_recast)[i % other_recast->size()]);
    }

    return result;
  }

  // Utility
  size_t size() const { return _storage.size(); }
  T operator[](size_t i) const { return _storage[i]; }
  void clear() { _storage.clear(); }
  void push_back(T element) { _storage.push_back(element); }
  size_t dimensions() const {
    return dim;
  };
  size_t cardinality(size_t i) const { return 1; }
  size_t long_length() const { return size(); }

  // Subsetting, assignment, combining etc
  virtual geometry_vector_base* new_from_vector(std::vector<T> vec) const = 0;
  cpp11::external_pointer<geometry_vector_base> subset(cpp11::integers index) const {
    std::vector<T> new_storage;
    new_storage.reserve(size());
    for (R_xlen_t i = 0; i < index.size(); ++i) {
      if (index[i] == R_NaInt) {
        new_storage.push_back(T::NA_value());
      } else {
        new_storage.push_back(_storage[index[i] - 1]);
      }
    }
    return {new_from_vector(new_storage)};
  }
  cpp11::external_pointer<geometry_vector_base> copy() const {
    std::vector<T> new_storage;
    new_storage.reserve(size());
    new_storage.insert(new_storage.begin(), _storage.begin(), _storage.end());
    return {new_from_vector(new_storage)};
  }
  cpp11::external_pointer<geometry_vector_base> assign(cpp11::integers index, const geometry_vector_base& value) const {
    if (index.size() != value.size()) {
      cpp11::stop("Incompatible vector sizes");
    }
    if (typeid(*this) != typeid(value)) {
      cpp11::stop("Incompatible assignment value type");
    }

    const geometry_vector<T, dim>* value_recast = dynamic_cast< const geometry_vector<T, dim>* >(&value);

    std::vector<T> new_storage(_storage);
    int max_size = *std::max_element(index.begin(), index.end());
    if (max_size > new_storage.size()) {
      new_storage.reserve(max_size);
      for (int j = new_storage.size(); j < max_size; ++j) {
        new_storage.push_back(T::NA_value());
      }
    }
    for (R_xlen_t i = 0; i < index.size(); ++i) {
      new_storage[index[i] - 1] = (*value_recast)[i];
    }
    return {new_from_vector(new_storage)};
  }
  cpp11::external_pointer<geometry_vector_base> combine(cpp11::list_of< cpp11::external_pointer<geometry_vector_base> > extra) const {
    std::vector<T> new_storage(_storage);

    for (R_xlen_t i = 0; i < extra.size(); ++i) {
      geometry_vector_base* candidate = extra[i].get();
      if (typeid(*this) != typeid(*candidate)) {
        cpp11::stop("Incompatible vector types");
      }
      const geometry_vector<T, dim>* candidate_recast = dynamic_cast< const geometry_vector<T, dim>* >(candidate);
      for (size_t j = 0; j < candidate_recast->size(); ++j) {
        new_storage.push_back((*candidate_recast)[j]);
      }
    }

    return {new_from_vector(new_storage)};
  }

  // Self-similarity
  cpp11::external_pointer<geometry_vector_base> unique() const {
    std::vector<T> new_storage;
    bool NA_seen = false;
    for (auto iter = _storage.begin(); iter != _storage.end(); ++iter) {
      if (!iter->is_valid()) {
        if (!NA_seen) {
          new_storage.push_back(T::NA_value());
          NA_seen = true;
        }
        continue;
      }
      if (std::find(new_storage.begin(), new_storage.end(), *iter) == new_storage.end()) {
        new_storage.push_back(*iter);
      }
    }

    return {new_from_vector(new_storage)};
  };
  cpp11::writable::logicals duplicated() const {
    std::vector<T> uniques;
    cpp11::writable::logicals dupes;
    dupes.reserve(size());
    bool NA_seen = false;
    for (auto iter = _storage.begin(); iter != _storage.end(); ++iter) {
      if (!iter->is_valid()) {
        if (!NA_seen) {
          dupes.push_back(TRUE);
          NA_seen = true;
        }
        continue;
      }
      if (std::find(uniques.begin(), uniques.end(), *iter) == uniques.end()) {
        uniques.push_back(*iter);
        dupes.push_back(FALSE);
      } else {
        dupes.push_back(TRUE);
      }
    }

    return dupes;
  }
  int any_duplicated() const {
    int anyone = -1;
    bool NA_seen = false;;
    int i = 0;
    for (auto iter = _storage.begin(); iter != _storage.end(); ++iter) {
      if (!_storage[i].is_valid()) {
        if (NA_seen) {
          anyone = i;
          break;
        }
        NA_seen = true;
      } else if (std::find(iter + 1, _storage.end(), *iter) != _storage.end()) {
        anyone = true;
        break;
      }
      ++i;
    }

    return anyone;
  }
  cpp11::writable::integers match(const geometry_vector_base& table) const {
    cpp11::writable::integers results;
    results.reserve(size());

    if (typeid(*this) != typeid(table)) {
      for (size_t i = 0; i < size(); ++i) {
        results.push_back(R_NaInt);
      }
      return results;
    }

    const geometry_vector<T, dim>* table_recast = dynamic_cast< const geometry_vector<T, dim>* >(&table);

    int NA_ind = -1;

    std::vector<T> lookup;
    lookup.reserve(table_recast->size());
    for (size_t i = 0; i < table_recast->size(); ++i) {
      if (NA_ind == -1 && !(*table_recast)[i]) {
        NA_ind = i;
      }
      lookup.push_back((*table_recast)[i]);
    }

    cpp11::writable::integers result;
    result.reserve(size());
    auto start = lookup.begin();
    for (auto iter = _storage.begin(); iter != _storage.end(); ++iter) {
      if (!iter->is_valid()) {
        if (NA_ind == -1) {
          result.push_back(R_NaInt);
        } else {
          result.push_back(NA_ind + 1);
        }
        continue;
      }
      auto match = std::find(lookup.begin(), lookup.end(), *iter);
      if (match == lookup.end()) {
        result.push_back(R_NaInt);
      } else {
        result.push_back((match - start) + 1);
      }
    }

    return result;
  }
  cpp11::writable::logicals is_na() const {
    cpp11::writable::logicals result;
    result.reserve(size());

    for (auto iter = _storage.begin(); iter != _storage.end(); ++iter) {
      result.push_back((Rboolean) !(*iter));
    }

    return result;
  }
  bool any_na() const {
    for (auto iter = _storage.begin(); iter != _storage.end(); ++iter) {
      if (!(*iter)) {
        return true;
      }
    }
    return false;
  }

  // Predicates
  cpp11::writable::logicals is_degenerate() const {
    cpp11::writable::logicals result;
    result.reserve(_storage.size());
    for (size_t i = 0; i < _storage.size(); ++i) {
      result.push_back(FALSE);
    }
    return result;
  }
};