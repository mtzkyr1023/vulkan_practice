#ifndef _MK_EXCEPTION_H_
#define _MK_EXCEPTION_H_

#include <exception>


class MkException : public std::exception {
public:
	MkException(const char* message) :
		std::exception(message) {
	}
};


#endif