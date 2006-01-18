/*
IoBind Library License:
--------------------------

The zlib/libpng License Copyright (c) 2003 Jonathan de Halleux

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution
*/


#ifndef IOBIND_PARSER_UTILS_HPP
#define IOBIND_PARSER_UTILS_HPP

#include <boost/spirit.hpp>
#include <boost/call_traits.hpp>
#include <string>

namespace iobind{
namespace parser{
namespace detail{

template<
	typename Container,
	typename Policy
> 
class append_with_policy
{
public:
    append_with_policy( Container& container_, Policy const& policy_)
        : m_container(container_), m_policy(policy_)
    {};

    // the method called by the parser    
    template <typename IteratorT>
    void operator()(IteratorT const& first, IteratorT const& last) const
    {
		m_container.insert(m_container.end(), m_policy.encode( std::string(first, last) ) );
    }

private:
    Container& m_container;
	Policy const& m_policy;
};

template<
	typename Container,
	typename Policy
> 
class insert_with_policy
{
public:
    insert_with_policy( size_t& index_, Container& container_, Policy const& policy_)
        : m_index(index_), m_container(container_), m_policy(policy_)
    {};

    // the method called by the parser    
    template <typename IteratorT>
    void operator()(IteratorT const& first, IteratorT const& last) const
    {
		if (m_index < m_container.size())
			m_container[m_index++]=m_policy.encode( std::string(first, last) );
#ifdef _DEBUG
		else
			std::cerr<<"insert_with_policy: could not add data"<<std::endl;
#endif
	}

private:
	size_t& m_index;
    Container& m_container;
	Policy const& m_policy;
};

template<
	typename Pair,
	typename FirstPolicy,
	typename SecondPolicy
> 
class assign_pair_with_policy
{
public:
 
	explicit assign_pair_with_policy( 
		Pair& pair_, 
		FirstPolicy const& first_policy_,
		SecondPolicy const& second_policy_,
		std::string const& first_,
		std::string const& second_
		)
        : 
		m_pair(pair_), 
		m_first_policy(first_policy_),
		m_second_policy(second_policy_),
		m_first(first_),
		m_second(second_)
    {};

    // the method called by the parser    
    template <typename IteratorT>
    void operator()(IteratorT first, IteratorT last) const
    {
		m_pair=Pair(
			m_first_policy.encode(m_first.c_str()),
			m_second_policy.encode(m_second.c_str())
			);
    }

private:
    Pair& m_pair;
	FirstPolicy const& m_first_policy;
	SecondPolicy const& m_second_policy;
	std::string const& m_first;
	std::string const& m_second;
};

class concat_string
{
public:
    // key_ and val_ should point to the string modified in keyvalue_grammar
    // kvc_ is the map of key - values
    concat_string( std::ostream& out_)
        : out(out_)
    {  };

    // the method called by the parser    
    template <typename IteratorT>
    void operator()(IteratorT first, IteratorT last) const
    {
		out<<std::string(first,last);
    }

	template <typename IteratorT>
	void operator()(IteratorT single) const
	{
		out<<single;
	}
private:
    std::ostream& out;
};

class concat_symbol
{
public:
    // key_ and val_ should point to the string modified in keyvalue_grammar
    // kvc_ is the map of key - values
    concat_symbol( std::ostream& out_)
        : out(out_)
    {  };

    // the method called by the parser    
	void operator()(std::string const& str) const
	{
		out<<str;
	}
private:
    std::ostream& out;
};

class concat_pre_post_symbol
{
public:
    // key_ and val_ should point to the string modified in keyvalue_grammar
    // kvc_ is the map of key - values
    concat_pre_post_symbol( std::ostream& out_, std::string const& pre_, std::string const& post_)
        : m_out(out_),m_pre(pre_), m_post(post_)
    {  };

    // the method called by the parser    
	void operator()(std::string const& str_) const
	{
		m_out<<m_pre<<str_<<m_post;
	}
private:
    std::ostream& m_out;
	std::string m_pre;
	std::string m_post;
};

};//details
};//parser
};//iobind

#endif


