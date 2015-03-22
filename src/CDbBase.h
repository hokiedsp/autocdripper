#pragma once

class CDbBase
{
    virtual ~CDbBase() {}

    /**
     * @brief Return true if Database can be searched for a CD info
     * @return true if database can be searched for a CD release
     */
    bool IsReleaseDb()=0;

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    bool IsImageDb()=0;

    bool CanSearchByArtistTitle()=0;
    bool CanSearchByCDDBID()=0;
    bool CanSearchByMBID()=0;
    bool CanSearchByUPC()=0;

    /**
     * @brief Return true if DB depends on MusicBrainz results
     * @return
     */
    bool DependsOnMusicBrainz()=0;
}
