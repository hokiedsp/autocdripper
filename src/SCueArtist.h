#pragma once

struct SCueArtist
{
    std::string name;   // artist name
    std::string joiner; // joining phrase if more artist follows
};

typedef std::vector<SCueArtist> SCueArtists;
