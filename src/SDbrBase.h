#pragma once

#include <string>

#include "SCueSheet.h"

/** CD Database Record structure - SCueSheet with DbType()
 */
struct SDbrBase : SCueSheet
{
	/** Name of the database the record was retrieved from
	 *
	 *  @return Name of the database
	 */
	virtual std::string SourceDatabase() const=0;

	friend std::ostream& operator<<(std::ostream& stdout, const SDbrBase& obj);
};

/** Overloaded stream insertion operator to output the content of SDbrCDDB
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
inline std::ostream& operator<<(std::ostream& os, const SDbrBase& o)
{
	os << dynamic_cast<const SCueSheet&>(o);
	return os;
}
