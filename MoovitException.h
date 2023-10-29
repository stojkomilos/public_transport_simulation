#pragma once

#include<exception>
#include<string>

// exceptions are used in 2 places: reading an input file (option 1 when starting a program)
//									finding a route

class MoovitException: public std::exception
{
public:
	MoovitException() = delete; // for safety
	MoovitException(const char* s)
		: exception(s)
	{}

};

class MoovitExceptionFile : public MoovitException
{
public:
	MoovitExceptionFile(const char* s)
		: MoovitException(s)
	{}
};

class MoovitExceptionBusLine : public MoovitException
{
public:
	MoovitExceptionBusLine(const char* s)
		: MoovitException(s)
	{}
};

class MoovitExceptionBusStop : public MoovitException
{
public:
	MoovitExceptionBusStop(const char* s)
		: MoovitException(s)
	{}
};

class MoovitExceptionNoRoute : public MoovitException
{
public:
	MoovitExceptionNoRoute(const char* s)
		: MoovitException(s)
	{}
};

class MoovitExceptionInvalidRouteType : public MoovitException
{
public:
	MoovitExceptionInvalidRouteType(const char* s)
		: MoovitException(s)
	{}
};
