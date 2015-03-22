#include "CSinkWavPack.h"

// http://www.wavpack.com/lib_use.txt

#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <sstream>

using std::string;
using std::runtime_error;

/* WavpackBlockOutput function
 */    
static int write_block (void *id, void *data, int32_t length)
{
	CSinkWavPack &sink = *(CSinkWavPack*) id;

	try
	{
		sink.WriteFile(data,length);
	}
	catch (...)
	{
		return 0;
	}

	return 1;
}

#define CLEAR_(destin) memset (&destin, 0, sizeof (destin));

CSinkWavPack::CSinkWavPack(const string &path) 
	: CSinkBase(path), first_block_size(0)
{
	// initialize its member variables
	CLEAR_(config);

	// set configuration for standard cd digital audio format
	config.bytes_per_sample = 2;	// 16-bit integer
	config.bits_per_sample = 16;
	config.channel_mask = 3;		// stereo
	config.num_channels = 2;
	config.sample_rate = 44100;
	config.flags = CONFIG_VERY_HIGH_FLAG; // CONFIG_HIGH_FLAG
	/* other WavpackConfig fields:
	 * - bitrate (float)
	 * - shaping_weight (float)
    * - qmode (int)
    * - xmode (int)
    * - float_norm_exp (int)
    * - block_samples (int32_t)
    * - extra_flags (int32_t)
    * - md5_checksum [16] (unsigned char)
    * - md5_read (unsigned char)
    * - num_tag_strings (int)
    * - tag_strings (char **)
    */
	
	// create the WavPack writer's context
	wpc = WavpackOpenFileOutput(write_block, this, NULL);
	if (!wpc) throw (runtime_error("Could not create a wavpack context."));

}	

CSinkWavPack::~CSinkWavPack()
{
	// Close the context
	WavpackCloseFile(wpc);
}

/* Write a the header for a WAV file. */
void CSinkWavPack::WritePreamble(const uintptr_t sign)
{
    if (sign!=GetLockSign_())
        throw(runtime_error("The calling thread must call Lock() first; it has not attained exclusive access."));

	if (!WavpackSetConfiguration (wpc, &config, -1))
		throw(runtime_error("Failed to set wavpack configuration."));

	//  4. prepare for packing with WavpackPackInit()
	WavpackPackInit(wpc);
}

int CSinkWavPack::WriteFrame(const int16_t* data, const size_t framesize, const uintptr_t sign)
{
    if (sign!=GetLockSign_())
        throw(runtime_error("The calling thread must call Lock() first; it has not attained exclusive access."));

    // copy data to the sample buffers as a 32-bit data
	buffer.assign(data,data+framesize);
	
	// 5. actually compress audio and write blocks with WavpackPackSamples()
	if ( !WavpackPackSamples(wpc, buffer.data(), framesize/2) )
		throw(runtime_error("Failed to set wavpack configuration."));
	
	return framesize;
}

void CSinkWavPack::WritePostamble(const uintptr_t sign)
{
/*
  6. flush final samples into blocks with WavpackFlushSamples()
  10. optionally append metadata tag with functions in next section
  11. optionally update number of samples with WavpackUpdateNumSamples()}
  
	If the duration is not known then pass -1. In the case that the size is not
	known (or the writing is terminated early) then it is suggested that the
	application retrieve the first block written and let the library update the
	total samples indication. A function is provided to do this update.
*/
    if (sign!=GetLockSign_())
        throw(runtime_error("The calling thread must call Lock() first; it has not attained exclusive access."));

    // we're now done with any WavPack blocks, so flush any remaining data
	if (!WavpackFlushSamples (wpc))
		throw(runtime_error("WavPack: Failed to flush samples."));

	// to see if we need to create & write a tag 
	// (which is NOT stored in regular WavPack blocks)
	WriteTags_();

	// At this point we're done writing to the output files. However, in some
	// situations we might have to back up and re-write the initial blocks.
	// Currently the only case is if we're ignoring length or inputting raw pcm data.
	UpdatePreamble_();
}

void CSinkWavPack::WriteTags_()
{
	// loop through all the tags and write them 
	for (const STagAPEv2 *tag = (const STagAPEv2*)tags.ReadFirstTag(); tag; tag = (const STagAPEv2*)tags.ReadNextTag())
	{
		int res;
		
		// append the tag to the file
		switch (tag->enc)
		{
		case APEv2_ENC_UTF8:
			res = WavpackAppendTagItem (wpc, tag->key.c_str(), tag->val.data(), tag->val.size());
			break;
		case APEv2_ENC_BINARY:
			res = WavpackAppendBinaryTagItem (wpc, tag->key.c_str(), tag->val.data(), tag->val.size());
			break;
		default: // APEv2_ENC_EXTREF not supported
			printf("WavPack: Skipping %s tag, which uses an unsupported encoding.", tag->key.c_str());
		}
		
		if (!res)
			throw(runtime_error("WavPack: Failed to write tags."));
	}
}

void CSinkWavPack::UpdatePreamble_()
{
	char *block_buff = new char [first_block_size];

	if (!block_buff) throw(runtime_error("Couldn't update WavPack header with actual length!!"));

	try // try block to make sure the block_buff gets deleted
	{
		// grab the first block data
		SeekFile_(0, SEEK_SET);
		ReadFile_(block_buff,first_block_size);

		// check for the 4-character file header
		if (strncmp (block_buff, "wvpk", 4)) throw;

		// update the first block to include the final sample size
		WavpackUpdateNumSamples (wpc, block_buff);

		// re-write the first block data
		SeekFile_(0, SEEK_SET);
		WriteFile_(block_buff,first_block_size);
	}
	catch (...)	// delete the buffer before throwing the error
	{
		delete[](block_buff);
		throw(runtime_error("couldn't update WavPack header with actual length!!"));
	}

	// done, delete teh buffer
	delete[](block_buff);
}

size_t CSinkWavPack::WriteFile(const void *buf, const size_t N)
{
	// perform the regular write op
	size_t bcount = CSinkBase::WriteFile_(buf,N);
	
	// if first block, store its size for later use
	if (!first_block_size) first_block_size = bcount;
	
	return bcount;
}

/**
 * @brief returns true if cuesheet can be embedded
 * @return always false
 */
bool CSinkWavPack::CueSheetEmbeddable() { return true; }

/**
 * @brief Add "cuesheet" tag entry to the output file
 * @param[in] reference to the cuesheet
 * @throw std::runtime_error if ISink instance does not support embedded
 *        cuesheet (i.e., CueSheetEmbeddable() returns false)
 */
void CSinkWavPack::SetCueSheet(const SCueSheet& cuesheet)
{
    std::ostringstream os ;
    os << cuesheet;
    tags.AppendTag("cuesheet", os.str().c_str());
}
