#pragma once

#include <string>
#include <deque>
#include <ostream>

#define CUE_FILETYPE_BINARY   0
#define CUE_FILETYPE_MOTOROLA 1
#define CUE_FILETYPE_AIFF     2
#define CUE_FILETYPE_WAVE     3
#define CUE_FILETYPE_MP3      4

#define CUE_TRACKTYPE_AUDIO      0 // – Audio/Music (2352)
#define CUE_TRACKTYPE_CDG        1 // – Karaoke CD+G (2448)
#define CUE_TRACKTYPE_MODE1_2048 3 // – CDROM Mode1 Data (cooked)
#define CUE_TRACKTYPE_MODE1_2352 4 // – CDROM Mode1 Data (raw)
#define CUE_TRACKTYPE_MODE2_2336 5 //– CDROM-XA Mode2 Data
#define CUE_TRACKTYPE_MODE2_2352 6 //– CDROM-XA Mode2 Data
#define CUE_TRACKTYPE_CDI_2336   7 //– CDI Mode2 Data
#define CUE_TRACKTYPE_CDI_2352   8 //– CDI Mode2 Data

#define CUE_TRACKFLAG_DCP 0x01  // – Digital copy permitted
#define CUE_TRACKFLAG_4CH 0x02  // – Four channel audio
#define CUE_TRACKFLAG_PRE 0x04  // – Pre-emphasis enabled (audio tracks only)
#define CUE_TRACKFLAG_SCMS 0x08 // – Serial copy management system (not supported by all recorders)
#define CUE_TRACKFLAG_DATA 0x10 // - Data (non-audio) CD

enum cdtimeunit_t {CDTIMEUNIT_SECONDS, CDTIMEUNIT_SECTORS, CDTIMEUNIT_WORDS, CDTIMEUNIT_BYTES};

struct SCueTrackIndex
{
	int number;  // - Index number (0-99), 0=pregap, 1=track start time
	size_t time; // - Starting time of the track in sectors
	
	SCueTrackIndex(const int n, const size_t t);
	SCueTrackIndex(const size_t t);
	~SCueTrackIndex();
	
	friend std::ostream& operator<<(std::ostream& stdout, const SCueTrackIndex& obj);
};

struct SCueTrack
{
	int number;   // - Track number (1-99)
	int datatype; // – Track datatype, one of CUE_TRACKTYPE_XXX

	int Flags;              // bit-OR combination of CUE_TRACKFLAG_XXX
	std::string Title;		// album title (80-char max)
	std::string Performer;	// performer (80-char long max)
	std::string Songwriter;	// name of songwriter (80-char max)
	std::string ISRC;	// “International Standard Recording Code” 12 character long
	std::deque<std::string> Rems;	// comments on track

	size_t Pregap;    // track pregap in # of sectors (i.e., frames)
	size_t Postgap;	// track postgap in # of sectors (i.e., frames)
	
	std::deque<SCueTrackIndex> Indexes;

	SCueTrack(const int number, const int type = CUE_TRACKTYPE_AUDIO);
	virtual ~SCueTrack();
	
	/** Returns the number of indexes for the track
	 */
	size_t NumberOfIndexes() const;
	
	/** Add new index and return reference to it. If index already exists, returns
	 *  the existing index object.
	 *
	 *  @param[in] Index number to add (0-99). Throws error if out of range value.
     *  @param[in] Index time in sectors. Only applied if non-zero.
     *  @return    Reference to the created/found SCueTrackIndex object
	 */
	SCueTrackIndex& AddIndex(const int number=1, const size_t time=0);
	
	/** Delete an index.
	 *
	 *  @param[in] Index number to delete (0-99).
	 */
	void DeleteIndex(const int number);
		
	/** Returns the starting time of the track in sector index
	 *
	 *  @return Track's starting time in sector index
	 */
	size_t StartTime(cdtimeunit_t units=CDTIMEUNIT_SECTORS) const;
	
	/** Check ISRC (International Standard Recording Code) validity. The ISRC must
	 *  be 12 characters in length. The first five characters are alphanumeric, but
	 *  the last seven are numeric only.
	 *
	 *  @return     true if ISRC empty or meets the rule.
	 */
	bool CheckISRC() const;
	
	friend std::ostream& operator<<(std::ostream& stdout, const SCueTrack& obj);

};

struct SCueSheet
{
public:
	std::string Catalog;		// 13-digit Media Catalog Number
	std::string CdTextFile;	// path to CD Text File
	std::string FileName;	// path to CD Data File
	int FileType;           // one of CUE_FILETYPE_XXX
	std::string Performer;	// performer (80-char long max)
	std::deque<std::string> Rems;	// comments on the disc 
	std::string Songwriter;	// name of songwriter (80-char max)
	std::string Title;		// album title (80-char max)

	std::deque<SCueTrack> Tracks;

	SCueSheet(const int type=CUE_FILETYPE_WAVE);
	virtual ~SCueSheet();

	/** Returns the number of indexes for the track
	 */
	size_t NumberOfTracks() const;
	
	/** Populate Tracks with num_tracks with specified track type. All existing
	 *  tracks are removed.
	 *
	 *  @param[in] Number of tracks to add (1-99). Throws error if out of range value.
	 */
	void AddTracks(const size_t num_tracks, const int type=CUE_TRACKTYPE_AUDIO);
	
	/** Add new track and return reference to it. If track already exists, returns
	 *  the existing track object.
	 *
	 *  @param[in] Track number to add (1-99). Throws error if out of range value.
	 *  @return    Reference to the created/found SCueTrack object
	 */
	SCueTrack& AddTrack(const int number, const int type=CUE_TRACKTYPE_AUDIO);
	
	/** Delete a track.
	 *
	 *  @param[in] Index number to delete (1-99).
	 */
	void DeleteTrack(const int number);

	/** Check Catalog number validity. The catalog number must be 13 digits long
	 *  and is encoded according to UPC/EAN rules.
	 *
	 *  @return     true if Catalog is empty or meets the rule.
	 */
	bool CheckCatalog() const;

	friend std::ostream& operator<<(std::ostream& stdout, const SCueSheet& obj);
};

/** Overloaded stream insertion operator to output the content of SCueSheet
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
std::ostream& operator<<(std::ostream& stdout, const SCueSheet& obj);

/** Overloaded stream insertion operator to output the content of SCueTrack
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
std::ostream& operator<<(std::ostream& stdout, const SCueTrack& obj);

/** Overloaded stream insertion operator to output the content of SCueTrackIndex
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
std::ostream& operator<<(std::ostream& stdout, const SCueTrackIndex& obj);

