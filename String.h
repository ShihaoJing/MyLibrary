//
// Created by Shihao Jing on 9/1/17.
//

#ifndef MYLIBRARY_STRING_H
#define MYLIBRARY_STRING_H

#include <memory>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>

class String {
 public:
  friend std::ostream& operator<<(std::ostream &os, const String &s);
  friend std::istream& operator>>(std::istream &is, String &s);
  friend bool operator==(const String& lhs, const String& rhs) {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  friend bool operator!=(const String& lhs, const String& rhs) {
    return !(lhs == rhs);
  }

  friend String operator+( const String& s1, const String& s2 );
  friend String operator+( const String& s, char c );
  friend String operator+( char c, const String& s );

  char& operator[] (std::size_t index) {
    if (index >= size()) {
      std::cerr << "Invalid index in myString::operator[]. Aborting...\n";
      exit(-1);   // exit immediately. This is BAD -- somebody has to
      // first deallocate the strings we allocated, or else
      // we have a memory leak!
    }
    return *(elements + index);
  }

  String& operator+=(const String &s) {
    std::for_each(s.begin(), s.end(), [this](char c) {
      check_n_alloc();
      alloc.construct(first_free++, c);
    });
    return *this;
  }

  String() : elements(nullptr), first_free(nullptr), cap(nullptr) {}
  String(std::size_t n) : elements(nullptr), first_free(nullptr), cap(nullptr) {
    reserve(n);
  }

  String(const char *s) {
    std::cout << "String(const char *s)" << std::endl;
    char *p = const_cast<char*>(s);
    while (*p)
      ++p;
    range_initializer(s, p);
  }

  String(const String &s) {
    std::cout << "String(const String &s) " << std::endl;
    range_initializer(s.elements, s.first_free);
  }

  String(String &&s) noexcept : elements(s.elements), first_free(s.first_free), cap(s.cap) {
    std::cout << "String(String &&s) " << std::endl;
    s.elements = s.first_free = s.cap = nullptr;
  }

  String& operator=(const String &s) {
    std::cout << "String& operator=(const String &s) " << std::endl;
    if (this != &s) {
      auto newstr = alloc_n_copy(s.elements, s.first_free);
      free();
      elements = newstr.first;
      first_free = cap = newstr.second;
    }
    return *this;
  }

  String& operator=(String &&s) {
    if (this != &s) {
      free();
      elements = s.elements;
      first_free = s.first_free;
      cap = s.cap;
      s.elements = s.first_free = s.cap = nullptr;
    }
    return *this;
  }

  ~String() {
    free();
  }

  std::size_t size() const { return first_free - elements; }
  std::size_t length() const { return size(); }
  std::size_t capacity() const { return cap - elements; }
  void reserve(std::size_t new_cap) {
    if (new_cap <= capacity())
      return;
    alloc_n_move(new_cap);
  }
  void push_back(char c) {
    check_n_alloc();
    alloc.construct(first_free++, c);
  }
  const char* c_str() const { return cbegin(); }
  char* begin() const { return elements; }
  char* end() const { return first_free; }
  const char* cbegin() const { return elements; }
  const char* cend() const { return first_free; }

 private:
  void alloc_n_move(std::size_t new_cap) {
    auto first = alloc.allocate(new_cap);
    auto last = std::uninitialized_copy(std::make_move_iterator(begin()),
                                        std::make_move_iterator(end()), first);
    free();
    elements = first;
    first_free = last;
    cap = elements + new_cap;
  }

  std::pair<char*, char*>
  alloc_n_copy(const char *b, const char *e) {
    auto data = alloc.allocate(e - b);
    return {data, std::uninitialized_copy(b, e, data)};
  };

  void check_n_alloc() {
    if (size() == capacity())
      reallocate();
  }

  void reallocate() {
    auto new_cap = size() ? 2 * size() : 1;
    alloc_n_move(new_cap);
  }

  void range_initializer(const char* b, const char* e) {
    auto newdata = alloc_n_copy(b, e);
    elements = newdata.first;
    first_free = cap = newdata.second;
  }

  void free() {
    if (elements) {
      std::for_each(elements, first_free, [this] (char &c) { alloc.destroy(&c); });
      alloc.deallocate(elements, cap - elements);
    }
  }
  char *elements;
  char *first_free;
  char *cap;

  std::allocator<char> alloc;
};

std::ostream& operator<<(std::ostream &os, const String &s) {
  std::for_each(s.elements, s.first_free, [&os](char &c) { os << c; });
  return os;
}

std::istream& operator>>(std::istream &is, String &s) {
  s.free();
  char c;
  while (is >> c) {
    s.check_n_alloc();
    s.alloc.construct(s.first_free++, std::move(c));
  }
  return is;
}

String operator+(const String& s1, const String& s2) {
  String s(s1.size() + s2.size());
  std::for_each(s1.begin(), s1.end(), [&s](char c) {s.alloc.construct(s.first_free++, c);});
  std::for_each(s2.begin(), s2.end(), [&s](char c) {s.alloc.construct(s.first_free++, c);});
  return s;
}

String operator+(const String& s, char c) {
  String new_s(s);
  new_s.push_back(c);
  return new_s;
}

String operator+(char c, const String& s) {
  String new_s(s.size() + 1);
  new_s.first_free++;
  std::for_each(s.begin(), s.end(), [&new_s](char c) {new_s.alloc.construct(new_s.first_free++, c);});
  new_s.alloc.construct(new_s.elements, c);
  return new_s;
}

void String_Test() {
  String s1; // s1 == ""
  assert(s1.length() == 0);

  String s2("hi");  // s2 == "hi"
  assert(s2.length() == 2);

  String s3(s2);  // s3 == "hi"
  assert(s3.length() == 2);

  assert(s3[0] == 'h');
  assert(s3[1] == 'i');

  s1 = s2;  // s1 == "hi"

  assert(s1 == s2);

  s3 = "bye";  // s3 == "bye"
  assert(s3.length() == 3);
  assert(s3[0] == 'b');
  assert(s3[1] == 'y');
  assert(s3[2] == 'e');

  s1 += "re";  // s1 == "hire"
  assert(s1 == "hire");

  s1 += "d"; // s1 == "hired"
  assert(not (s1 == "hire"));

  std::cout << "SUCCESS" << std::endl;
}

#endif //MYLIBRARY_STRING_H
