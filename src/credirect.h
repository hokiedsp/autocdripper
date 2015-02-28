#pragma once

#include <iostream>

struct cout_redirect
{
	cout_redirect( std::streambuf * new_buffer ) 
		: old( std::cout.rdbuf( new_buffer ) )
	{ }

	~cout_redirect( ) 
	{
		std::cout.rdbuf( old );
	}

private:
	std::streambuf * old;
};

struct cerr_redirect
{
	cerr_redirect( std::streambuf * new_buffer ) 
		: old( std::cerr.rdbuf( new_buffer ) )
	{ }

	~cerr_redirect( ) 
	{
		std::cerr.rdbuf( old );
	}

private:
	std::streambuf * old;
};

