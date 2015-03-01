#pragma once

struct SDbrBase;
struct SCueSheet;

/** Database Access Interface
 */
class IDatabase
{
public:
  virtual ~IDatabase() {}

  /** Perform a new query for the CD info in the specified drive with its
   *  tracks specified in the supplied cuesheet and its length. Previous query
   *  outcome discarded. After disc and its tracks are initialized, CDDB disc ID
   *  is computed. If the computation fails, function throws an runtime_error.
   *
   *  @param[in] CD-ROM device path
   *  @param[in] Cuesheet with its basic data populated
   *  @param[in] Length of the CD in sectors
   *  @param[in] If true, immediately calls Read() to populate disc records.
   *  @param[in] Network time out in seconds. If omitted or negative, previous value
   *             will be reused. System default is 10.
   *  @return    Number of matched records
   */
  virtual int Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const bool autofill=false, const int timeout=-1)=0;

  /** Return the discid string
   *
   *  @return discid string if Query was successful.
   */
  virtual std::string GetDiscId() const=0;


  /** Returns the number of matched records returned from the last Query() call.
   *
   *  @return    Number of matched records
   */
  virtual int NumberOfMatches() const=0;

  /** Returns the CD record ID associated with the specified genre. If no matching
   *  record is found, it returns -1.
   *
   *  @return Matching CD record ID.
   */
  virtual int MatchByGenre(const std::string &genre) const=0;

  /** Look up full disc information from CDDB server. It supports single record or
   *  multiple records if multiple entries were found by Query(). If the computation
   *  fails, function throws an runtime_error.
   *
   *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
   *             for all records.
   *  @param[in] Network time out in seconds. If omitted or negative, previous value
   *             will be reused. System default is 10.
   */
  virtual void Populate(const int recnum=-1, const int timeout=-1)=0;

  /** Retrieve the disc info from specified database record
   *
   *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
   *             is returned.
   *  @return    SDbrBase Pointer to newly created database record object. Caller is
   *             responsible for deleting the object.
   */
  virtual SDbrBase* Retrieve(const int recnum=0)=0;
};
