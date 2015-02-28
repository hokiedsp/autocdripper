#pragma once

#include <iostream>
#include <sstream>

struct cout_redirect
{
	cout_redirect()
	{
		newbuf = new std::stringbuf;
		orgbuf = std::cout.rdbuf(newbuf);
	}

	cout_redirect( std::streambuf * new_buffer ) 
		: newbuf(NULL), orgbuf( std::cout.rdbuf( new_buffer ) )
	{ }

	~cout_redirect( ) 
	{
		if (newbuf) delete newbuf;
		std::cout.rdbuf( orgbuf );
	}

private:
	std::streambuf * newbuf; // internal buffer, created only default-constructed
	std::streambuf * orgbuf; // original stream buffer
};

struct cerr_redirect
{
	cerr_redirect()
	{
		newbuf = new std::stringbuf;
		orgbuf = std::cerr.rdbuf(newbuf);
	}

	cerr_redirect( std::streambuf * new_buffer ) 
		: newbuf(NULL), orgbuf( std::cerr.rdbuf( new_buffer ) )
	{ }

	~cerr_redirect( ) 
	{
		if (newbuf) delete newbuf;
		std::cerr.rdbuf( orgbuf );
	}

private:
	std::streambuf * newbuf; // internal buffer, created only default-constructed
	std::streambuf * orgbuf; // original stream buffer
};

