#include "SoundBufferAiff.hpp"

#if defined(ACID_BUILD_MACOS)
#include <OpenAL/al.h>
#else
#include <al.h>
#endif

#include "Files/Files.hpp"
#include "Maths/Time.hpp"

namespace acid {
bool SoundBufferAiff::registered = Register(".aiff");

void SoundBufferAiff::Load(SoundBuffer *soundBuffer, const std::filesystem::path &filename) {
#if defined(ACID_DEBUG)
	auto debugStart = Time::Now();
#endif

	auto fileLoaded = Files::Read(filename);

	if (!fileLoaded) {
		Log::Error("SoundBuffer could not be loaded: ", filename, '\n');
		return;
	}

	//soundBuffer->SetBuffer(buffer);

#if defined(ACID_DEBUG)
	Log::Out("SoundBuffer ", filename, " loaded in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

void SoundBufferAiff::Write(const SoundBuffer *soundBuffer, const std::filesystem::path &filename) {
#if defined(ACID_DEBUG)
	auto debugStart = Time::Now();
#endif


#if defined(ACID_DEBUG)
	Log::Out("SoundBuffer ", filename, " written in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}
}
