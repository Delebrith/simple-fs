#pragma once

#include <exception>

class DiskException : public std::exception
{
	const char* msg;

	const char* what () const noexcept override {
		return msg;
	}

public:
	explicit DiskException(const char* msg) : msg(msg) {}
};
