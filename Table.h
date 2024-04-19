/*
 * File:		Table.h
 *
 * Description: This file contains the class definition for a hash table used for
 *				partion and disk managment.  At this point, a Table merely consists of a
 *				name and a type, neither of which you can change.
 */

# ifndef TABLE_H
# define TABLE_H 
# include <string>

class Symbol {
	typedef std::string string;
	string _name;
	Type _type;

public:
	int offset;

	Symbol(const string &name, const Type &type);
	const string &name() const;
	const Type &type() const;
};

# endif /* TABLE_H */
