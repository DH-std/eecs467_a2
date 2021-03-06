#ifndef EXCEPT_H
#define EXCEPT_H

#include <string>
#include <exception>

struct MyException : public std::exception
{
   std::string s;
   MyException(std::string ss) : s(ss) {}
   ~MyException() throw () {} // Updated
   const char* what() const throw() { return s.c_str(); }
};

#endif // EXCEPT_H
