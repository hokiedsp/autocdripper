#pragma once

#include <string>
#include <vector>
#include <functional>

class CDbMusicBrainz;

class IImageDatabase;
typedef std::vector<IImageDatabase*> IImageDatabasePtrVector;
typedef std::vector<std::reference_wrapper<IImageDatabase>> IImageDatabaseRefVector;
typedef std::vector<unsigned char> UByteVector;

class IImageDatabase
{
public:

    /** Destructor
     */
    virtual ~IImageDatabase(){}

    /** Specify the preferred coverart image width
     *
     *  @param[in] Preferred width of the image
     */
    virtual void SetPreferredWidth(const size_t &width)=0;

    /** Specify the preferred coverart image height
     *
     *  @param[in] Preferred height of the image
     */
    virtual void SetPreferredHeight(const size_t &height)=0;

    /**
     * @brief Clear all the matches from previous search
     */
    virtual void Clear()=0;

    /** If AllowQueryCD() returns true, Query() performs a new query for the CD info
     *  in the specified drive with its *  tracks specified in the supplied cuesheet
     *  and its length. Previous query outcome discarded. After disc and its tracks
     *  are initialized, CDDB disc ID is computed. If the computation fails, function
     *  throws an runtime_error.
     *
     *  @param[in] CD-ROM device path
     *  @param[in] Cuesheet with its basic data populated
     *  @param[in] Length of the CD in sectors
     *  @param[in] (Optional) UPC barcode
     *  @return    Number of matched records
     */
      virtual int Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const std::string cdrom_upc="")=0;

    /** If MayBeLinkedFromMusicBrainz() returns true, Query() performs a new
     *  query based on the MusicBrainz query results.
     *
     *  @param[in] MusicBrainz database object.
     *  @return    Number of matched records
     */
    virtual int Query(const CDbMusicBrainz &mbdb)=0;

    /** Returns the number of matched records returned from the last Query() call.
     *
     *  @return    Number of matched records
     */
    virtual int NumberOfMatches() const=0;

    /** Check if the query returned a front cover
     *
     *  @param[in]  record index (default=0)
     *  @return     true if front cover is found.
     */
    virtual bool Front(const int recnum=0) const=0;

    /** Check if the query returned a back cover
     *
     *  @param[in]  record index (default=0)
     *  @return     true if back cover is found.
     */
    virtual bool Back(const int recnum=0) const=0;

    /** Retrieve the front cover data.
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual UByteVector FrontData(const int recnum=0) const=0;

    /** Check if the query returned a front cover
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual UByteVector BackData(const int recnum=0) const=0;

    /** Get the URL of the front cover image
     *
     *  @param[in]  Record index (default=0)
     *  @return     URL string
     */
    virtual std::string FrontURL(const int recnum=0) const=0;

    /** Get the URL of the back cover image
     *
     *  @param[in]  Record index (default=0)
     *  @return     URL string
     */
    virtual std::string BackURL(const int recnum=0) const=0;
};
