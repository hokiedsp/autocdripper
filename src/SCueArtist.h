#pragma once

enum struct SCueArtistType { UNKNOWN, PERSON, GROUP };

struct SCueArtistNoJoiner
{
    std::string name;   // artist name
    SCueArtistType type;

    SCueArtistNoJoiner(const std::string &inname="",
                       const SCueArtistType intype=SCueArtistType::UNKNOWN) :
        name(inname), type(intype) {}
};

struct SCueArtist : SCueArtistNoJoiner
{
    std::string joiner; // joining phrase if more artist follows

    SCueArtist(const std::string &inname="",
               const std::string &injoiner="",
               const SCueArtistType intype=SCueArtistType::UNKNOWN) :
        SCueArtistNoJoiner(inname,intype), joiner(injoiner) {}
};

typedef std::vector<SCueArtist> SCueArtists;
