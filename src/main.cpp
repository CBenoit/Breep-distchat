#include <AL/al.h>    // OpenAL header files
#include <AL/alc.h>

#include <list>
#include <array>

#include <breep/network/tcp.hpp>
#include <flow_controller.hpp>

namespace cst {
	constexpr int frequency = 22050;
	constexpr int cap_size  = 8192;
}

using buffer_type = std::array<short, 2 * cst::frequency>;

BREEP_DECLARE_TYPE(buffer_type)

void sender();
void receiver();

int main(int argc,char* argv[])
{
	if (argc < 2) {
		std::cout << "Missing arguments\n";
		return 1;
	}

	if (std::string(argv[1]) == "sender") {
		sender();
	} else if (std::string(argv[1]) == "receiver") {
		receiver();
	} else {
		std::cout << "Unknown: " << argv[1] << '\n';
		return 1;
	}
	return 0;
}

void receiver() {


	ALCdevice* audioDevice = alcOpenDevice(nullptr);

	ALCcontext* audioContext = alcCreateContext(audioDevice,nullptr);
	alcMakeContextCurrent(audioContext);
	ON_SCOPE_EXIT {
		              alcMakeContextCurrent(nullptr);
		              alcDestroyContext(audioContext);
		              alcCloseDevice(audioDevice);
	              };


	std::list<ALuint> bufferQueue; // A quick and dirty queue of buffer objects

	ALuint audioBuffer[16], audioSource;

	alGenBuffers(16, audioBuffer);

	// Queue our buffers onto an STL list
	for (unsigned int ii : audioBuffer) {
		bufferQueue.push_back(ii);
	}

	alGenSources (1, &audioSource); // Create a sound source

	ALint availBuffers = 0;           // Buffers to be recovered
	ALuint myBuff;                    // The buffer we're using
	ALuint buffHolder[16];            // An array to hold catch the unqueued buffers

	breep::tcp::network network(1233);
	network.add_data_listener<buffer_type>([&](breep::tcp::netdata_wrapper<buffer_type>& value) {
		// Poll for recoverable buffers
		alGetSourcei(audioSource, AL_BUFFERS_PROCESSED, &availBuffers);
		if (availBuffers > 0) {

			alSourceUnqueueBuffers(audioSource, availBuffers, buffHolder);
			for (int ii = 0 ; ii < availBuffers ; ++ii) {
				// Push the recovered buffers back on the queue
				bufferQueue.push_back(buffHolder[ii]);
			}
		}

		// Stuff the captured data in a buffer-object
		if (!bufferQueue.empty()) { // We just drop the data if no buffers are available
			myBuff = bufferQueue.front();
			bufferQueue.pop_front();

			alBufferData(myBuff, AL_FORMAT_MONO16, value.data.data(), cst::cap_size * sizeof(short), cst::frequency);

			// Queue the buffer
			alSourceQueueBuffers(audioSource, 1, &myBuff);

			// Restart the source if needed
			// (if we take too long and the queue dries up,
			//  the source stops playing).
			ALint sState = 0;
			alGetSourcei(audioSource, AL_SOURCE_STATE,&sState);
			if (sState!=AL_PLAYING) {
				alSourcePlay(audioSource);
			}
		}
	});
	network.connect(boost::asio::ip::address_v4::loopback(), 1234);
	network.join();

	// Stop the sources
	alSourceStopv(1,&audioSource);
	alSourcei(audioSource,AL_BUFFER,0);

	alDeleteSources(1, &audioSource);
	alDeleteBuffers(16, audioBuffer);

}

void sender() {

	breep::tcp::network network(1234);
	network.set_log_level(breep::log_level::info);
	network.awake();

	// Request the default capture device with a half-second buffer
	ALCdevice* inputDevice = alcCaptureOpenDevice(nullptr, cst::frequency, AL_FORMAT_MONO16, cst::frequency/2);
	alcCaptureStart(inputDevice);

	buffer_type buffer; // A buffer to hold captured audio
	ALCint samplesIn = 0;             // How many samples are captured
	bool done = false;

	while (!done) {

		std::this_thread::sleep_for(std::chrono::microseconds(1));
		// Poll for captured audio
		alcGetIntegerv(inputDevice, ALC_CAPTURE_SAMPLES, 1, &samplesIn);

		if (samplesIn > cst::cap_size) {
			// Grab the sound
			alcCaptureSamples(inputDevice, buffer.data(), cst::cap_size);
			network.send_object(buffer);
		}
	}
	// Stop capture
	alcCaptureStop(inputDevice);
	alcCaptureCloseDevice(inputDevice);
}
