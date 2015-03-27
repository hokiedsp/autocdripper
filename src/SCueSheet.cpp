#include "SCueSheet.h"

#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include <regex>
#include <algorithm>

#include <cdio/sector.h>

#define TimeToString(time,mm,ss,ff) \
		(ss) = (time)/75; \
		(ff) = (time) - (ss)*75; \
		(mm) = (ss)/60; \
		(ss) -= (mm)*60

using std::string;
using std::runtime_error;
using std::deque;
using std::distance;
using std::begin;
using std::end;
using std::endl;
using std::setfill;
using std::setw;
using std::any_of;
using std::regex;
using std::regex_match;

SCueTrackIndex::SCueTrackIndex(const int n, const size_t t) : number(n), time(t) {}
SCueTrackIndex::SCueTrackIndex(const size_t t) : number(1), time(t) {}
SCueTrackIndex::~SCueTrackIndex() {};

//-----------------------------------------------------------------------------//

SCueTrack::SCueTrack(const int n, const int t) : number(n), datatype(t), Flags(0), Pregap(0), Postgap(0) {}
SCueTrack::~SCueTrack() {}
	
/** Returns the number of indexes for the track
 */
size_t SCueTrack::NumberOfIndexes() const
{
	return distance(begin(Indexes), end(Indexes));
}

/** Returns the starting time of the track in sector index
 *
 *  @return Track's starting time in sector index
 */
size_t SCueTrack::StartTime(cdtimeunit_t units) const
{
	size_t time;
	
	// look for the number
    deque<SCueTrackIndex>::const_iterator it;
	for (it = Indexes.begin();it!=Indexes.end() && (*it).number<1; it++);
	
	if (it==Indexes.end() || (*it).number!=1) time = 0;
	else time = (*it).time;
	
	switch (units)
	{
		case CDTIMEUNIT_SECONDS:
			return time/CDIO_CD_FRAMES_PER_SEC;
		case CDTIMEUNIT_SECTORS:
			return time;
		case CDTIMEUNIT_WORDS:
			return time*(CDIO_CD_FRAMESIZE_RAW/2);
		default: // case CDTIMEUNIT_BYTES:
			return time*CDIO_CD_FRAMESIZE_RAW;
	}
}

/** Add new index and return reference to it. If index already exists, returns
 *  the existing index object.
 *
 *  @param[in] Index number to add (0-99). Throws error if out of range value.
 *  @param[in] Index time in sectors. Only applied if non-zero.
 *  @return    Reference to the created/found SCueTrackIndex object
 */
SCueTrackIndex& SCueTrack::AddIndex(const int number, const size_t time)
{
	if (number<0 || number>99)
		throw (runtime_error("Track Index must be between 0 and 99."));

    deque<SCueTrackIndex>::reverse_iterator it;

	// look for the number from the end (more likely to append)
	for (it = Indexes.rbegin(); it!=Indexes.rend() && (*it).number>number; it++);
	
	// if found return it, o.w., create a new element
	if (it==Indexes.rend())	// smallest index
	{
		Indexes.emplace_front(number,time);
		return Indexes.front();
	}
	else if (number==(*it).number) // matching index found
	{
		SCueTrackIndex &index = *it;
		index.time = time;
		return index;
	}
	else // new index
	{
		return *Indexes.emplace(it.base(),number,time);
	}
}

/** Delete an index.
*
*  @param[in] Index number to delete (0-99).
*/
void SCueTrack::DeleteIndex(const int number)
{
	if (number<0 || number>99) return;

	// look for the number
    deque<SCueTrackIndex>::iterator it;
	for (it = Indexes.begin(); it!=Indexes.end() && (*it).number<number; it++);
	
	// if found delete it
	if (it!=Indexes.end() && number==(*it).number) Indexes.erase(it);
}

/** Check ISRC (International Standard Recording Code) validity. The ISRC must
 *  be 12 characters in length. The first five characters are alphanumeric, but
 *  the last seven are numeric only.
 *
 *  @return     true if ISRC empty or meets the rule.
 */
bool SCueTrack::CheckISRC() const
{
	// if nothing written, no problem
	if (ISRC.empty()) return true;
	
	regex e("^[a-zA-Z]{2}[a-zA-Z0-9]{3}[0-9]{7}$", std::regex_constants::basic);
	return regex_match (ISRC,e);
}

//-----------------------------------------------------------------------------//

SCueSheet::SCueSheet(const int type) : FileType(type) {}
SCueSheet::~SCueSheet() {}

/** Returns the number of indexes for the track
 */
size_t SCueSheet::NumberOfTracks() const { return distance(begin(Tracks), end(Tracks)); }

/** Populate Tracks with num_tracks with specified track type. Any existing
 *  track exceeding num_tracks will be deleted.
 *
 *  @param[in] Number of tracks to add (1-99). Throws error if out of range value.
 */
void SCueSheet::AddTracks(const size_t num_tracks, const int type)
{
	if (num_tracks<1 || num_tracks>99)
		throw (std::runtime_error("Number of tracks must be between 1 and 99."));

	// Start from scratch
	if (!Tracks.empty()) Tracks.clear();

	// Create all the tracks at once (with incorrect track number)
	Tracks.resize(num_tracks,SCueTrack(1,type));
	
	// Correct the track number
    deque<SCueTrack>::iterator it = Tracks.begin();
	for (size_t t = 2; t<=num_tracks; t++) (*(++it)).number = t;;
}

/** Add new track and return reference to it. If track already exists, returns
 *  the existing track object.
 *
 *  @param[in] Track number to add (1-99). Throws error if out of range value.
 *  @return    Reference to the created/found SCueTrack object
 */
SCueTrack& SCueSheet::AddTrack(const int number, const int type)
{
	if (number<1 || number>99)
		throw (std::runtime_error("Track must be between 1 and 99."));

    deque<SCueTrack>::reverse_iterator it;

	// look for the number from the end
	for (it = Tracks.rbegin(); it!=Tracks.rend() && (*it).number>number; it++);
	
	// if found return it, o.w., create a new element
	if (it==Tracks.rend())	// smallest track#
	{
		Tracks.emplace_front(number,type);
		return Tracks.front();
	}
	else if (number==(*it).number) // matching track found
	{
		SCueTrack &track = *it;
		track.datatype = type;
		return track;
	}
	else // new track
	{
		return *Tracks.emplace(it.base(),number,type);
	}
}

/** Delete a track.
 *
 *  @param[in] Index number to delete (1-99).
 */
void SCueSheet::DeleteTrack(const int number)
{
	if (number<1 || number>99) return;

	// look for the number
    deque<SCueTrack>::iterator it;
	for (it = Tracks.begin(); it!=Tracks.end() && (*it).number<number; it++);
	
	// if found delete it
	if (it!=Tracks.end() && number==(*it).number) Tracks.erase(it);
}

/** Check Catalog number validity. The catalog number must be 13 digits long
 *  and is encoded according to UPC/EAN rules.
 *
 *  @return     true if Catalog is empty or meets the rule.
 */
bool SCueSheet::CheckCatalog() const
{
	// if nothing written, no problem
	if (Catalog.empty()) return true;
	
	regex e("^[0-9]{12}$", std::regex_constants::basic);
	return regex_match(Catalog,e);
}

//-----------------------------------------------------------------------------//

/** Overloaded stream insertion operator to output the content of SCueSheet
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
std::ostream& operator<<(std::ostream& os, const SCueSheet& o)
{
	if (!o.Catalog.empty() && o.CheckCatalog()) os << "CATALOG " << o.Catalog << endl;

	if (!o.FileName.empty())
	{
		os << "FILE ";
		if ( any_of(o.FileName.begin(), o.FileName.end(), [](char ch){ return ch==' '; }) )
			os << '"' << o.FileName << '"';
		else
			os << o.FileName;
	
		switch (o.FileType)
		{
			case CUE_FILETYPE_BINARY:
				os << "BINARY";
				break;
			case CUE_FILETYPE_MOTOROLA:
				os << "MOTOROLA";
				break;
			case CUE_FILETYPE_AIFF:
				os << "AIFF";
				break;
			case CUE_FILETYPE_WAVE:
				os << "WAVE";
				break;
			case CUE_FILETYPE_MP3:
				os << "MP3";
			default:
				throw (runtime_error("Invalid FILE type."));
		}
		os << endl;
	}

	if (!o.CdTextFile.empty())
	{
		os << "CDTEXTFILE ";
		if ( any_of(o.CdTextFile.begin(), o.CdTextFile.end(), [](char ch){return ch==' ';}) )
			os << '"' << o.CdTextFile << '"';
		else
			os << o.CdTextFile;
		os << endl;
	}

	if (!o.Title.empty()) os << "TITLE " << o.Title << endl;
	if (!o.Performer.empty()) os << "PERFORMER " << o.Performer << endl;
	if (!o.Songwriter.empty()) os << "SONGWRITER " << o.Songwriter << endl;

    deque<string>::const_iterator itRem;
	for (itRem = o.Rems.begin();	itRem!=o.Rems.end(); itRem++)
		os << "REM " << *itRem << endl;

    deque<SCueTrack>::const_iterator itTrack;
	for (itTrack = o.Tracks.begin();	itTrack!=o.Tracks.end(); itTrack++)
		os << (*itTrack);

	return os;
}

/** Overloaded stream insertion operator to output the content of SCueTrack
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
std::ostream& operator<<(std::ostream& os, const SCueTrack& o)
{
	size_t mm,ss,ff;

	// convert to string
	os << "  TRACK " << setfill('0') << setw(2) << o.number << " ";
	switch (o.datatype)
	{
		case CUE_TRACKTYPE_AUDIO:
			os << "AUDIO";
			break;
		case CUE_TRACKTYPE_CDG:
			os << "CDG";
			break;
		case CUE_TRACKTYPE_MODE1_2048:
			os << "MODE1/2048";
			break;
		case CUE_TRACKTYPE_MODE1_2352:
			os << "MODE1/2352";
			break;
		case CUE_TRACKTYPE_MODE2_2336:
			os << "MODE2/2336";
			break;
		case CUE_TRACKTYPE_MODE2_2352:
			os << "MODE2/2352";
			break;
		case CUE_TRACKTYPE_CDI_2336:
			os << "CDI/2336";
			break;
		case CUE_TRACKTYPE_CDI_2352:
			os << "CDI/2352";
			break;
		default:
			throw (runtime_error("Invalid TRACK type."));
	}
	os << endl;

	if (o.Flags)
	{
		os << "    FLAGS";
		if (o.Flags & CUE_TRACKFLAG_DCP) os << " DCP";
		if (o.Flags & CUE_TRACKFLAG_4CH) os << " 4CH";
		if (o.Flags & CUE_TRACKFLAG_PRE) os << " PRE";
		if (o.Flags & CUE_TRACKFLAG_SCMS) os << " SCMS";
		if (o.Flags & CUE_TRACKFLAG_DATA) os << " DATA";
		os << endl;
	}

	if (!o.Title.empty()) os << "    TITLE " << o.Title << endl;
	if (!o.Performer.empty()) os << "    PERFORMER " << o.Performer << endl;
	if (!o.Songwriter.empty()) os << "    SONGWRITER " << o.Songwriter << endl;
	if (!o.ISRC.empty() && o.CheckISRC()) os << "    ISRC " << o.ISRC << endl;
	
    deque<string>::const_iterator itRem;
	for (itRem = o.Rems.begin();	itRem!=o.Rems.end(); itRem++)
		os << "    REM " << *itRem << endl;

	if (o.Pregap>0)
	{
		TimeToString(o.Pregap,mm,ss,ff);
		os << "    PREGAP " << setfill('0') << setw(2) << mm << ":" 
			<< setfill('0') << setw(2) << ss << ":" << setfill('0') << setw(2) << ff << endl;
	}
	if (o.Postgap>0)
	{
		TimeToString(o.Postgap,mm,ss,ff);
		os << "    POSTGAP " << setfill('0') << setw(2) << mm << ":" 
			<< setfill('0') << setw(2) << ss << ":" << setfill('0') << setw(2) << ff << endl;
	}

    deque<SCueTrackIndex>::const_iterator itIndex;
	for (itIndex = o.Indexes.begin(); itIndex!=o.Indexes.end(); itIndex++)
		os << (*itIndex);
	
	return os;
}

/** Overloaded stream insertion operator to output the content of SCueTrackIndex
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
std::ostream& operator<<(std::ostream& os, const SCueTrackIndex& o)
{
	size_t mm,ss,ff;

	// break down the time in mm:ss:ff format
	TimeToString(o.time,mm,ss,ff);
	
	// convert to string
	os << "    INDEX " << setfill('0') << setw(2) << o.number << " " 
		<< setfill('0') << setw(2) << mm << ":" 
		<< setfill('0') << setw(2) << ss << ":" 
		<< setfill('0') << setw(2) << ff << endl;
	
	return os;
}

