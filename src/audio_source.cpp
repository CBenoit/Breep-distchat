/*************************************************************************************
 * MIT License                                                                       *
 *                                                                                   *
 * Copyright (c) 2018 TiWinDeTea                                                     *
 *                                                                                   *
 * Permission is hereby granted, free of charge, to any person obtaining a copy      *
 * of this software and associated documentation files (the "Software"), to deal     *
 * in the Software without restriction, including without limitation the rights      *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell         *
 * copies of the Software, and to permit persons to whom the Software is             *
 * furnished to do so, subject to the following conditions:                          *
 *                                                                                   *
 * The above copyright notice and this permission notice shall be included in all    *
 * copies or substantial portions of the Software.                                   *
 *                                                                                   *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR        *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,          *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE       *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER            *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,     *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE     *
 * SOFTWARE.                                                                         *
 *                                                                                   *
 *************************************************************************************/

#include <AL/alc.h>
#include <AL/al.h>
#include <list>

#include "sound_def.hpp"
#include "audio_source.hpp"

struct audio_source_life_manager {
	audio_source_life_manager() {

		audioDevice = alcOpenDevice(nullptr);
		if (!audioDevice) {
			throw std::runtime_error("No audio device found.");
		}

		audioContext = alcCreateContext(audioDevice, nullptr);
		alcMakeContextCurrent(audioContext);

		alGenBuffers(16, audioBuffer);

		// Queue our buffers onto an STL list
		for (unsigned int ii : audioBuffer) {
			bufferQueue.push_back(ii);
		}
		alGenSources (1, &audioSource); // Create a sound source
	}

	~audio_source_life_manager() {

		alSourceStopv(1, &audioSource);
		alSourcei(audioSource, AL_BUFFER, 0);

		alDeleteSources(1, &audioSource);
		alDeleteBuffers(16, audioBuffer);

		alcMakeContextCurrent(nullptr);
		alcDestroyContext(audioContext);
		alcCloseDevice(audioDevice);
	}

	ALCdevice* audioDevice{};
	ALCcontext* audioContext{};
	ALuint audioBuffer[16]{}, audioSource{};
	std::list<ALuint> bufferQueue{}; // A quick and dirty queue of buffer objects
};

void audio_source::play(const sound_buffer_t& buffer) {
	static audio_source_life_manager aslm{};

	ALint availBuffers = 0;           // Buffers to be recovered
	ALuint myBuff;                    // The buffer we're using
	ALuint buffHolder[16];            // An array to hold catch the unqueued buffers


	// Poll for recoverable buffers
	alGetSourcei(aslm.audioSource, AL_BUFFERS_PROCESSED, &availBuffers);
	if (availBuffers > 0) {

		alSourceUnqueueBuffers(aslm.audioSource, availBuffers, buffHolder);
		for (int ii = 0 ; ii < availBuffers ; ++ii) {
			// Push the recovered buffers back on the queue
			aslm.bufferQueue.push_back(buffHolder[ii]);
		}
	}

	// Stuff the captured data in a buffer-object
	if (!aslm.bufferQueue.empty()) { // We just drop the data if no buffers are available
		myBuff = aslm.bufferQueue.front();
		aslm.bufferQueue.pop_front();

		alBufferData(myBuff, AL_FORMAT_MONO16, buffer.data(), cst::cap_size * sizeof(short), cst::frequency);

		// Queue the buffer
		alSourceQueueBuffers(aslm.audioSource, 1, &myBuff);

		// Restart the source if needed
		// (if we take too long and the queue dries up,
		//  the source stops playing).
		ALint sState = 0;
		alGetSourcei(aslm.audioSource, AL_SOURCE_STATE,&sState);
		if (sState!=AL_PLAYING) {
			alSourcePlay(aslm.audioSource);
		}
	}
}
