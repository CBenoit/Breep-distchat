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

#include "sound_sender.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <sound_sender.hpp>


sound_sender::sound_sender() noexcept {

	// Request the default capture device with a half-second buffer
	m_inputDevice = alcCaptureOpenDevice(nullptr, cst::frequency, AL_FORMAT_MONO16, cst::frequency / 2);
	alcCaptureStart(m_inputDevice);
}

sound_sender::sound_sender(sound_sender&& other) noexcept {
	m_inputDevice = other.m_inputDevice;
	other.m_inputDevice = nullptr;
}

sound_sender& sound_sender::operator=(sound_sender&& other) noexcept {
	m_inputDevice = other.m_inputDevice;
	other.m_inputDevice = nullptr;
	return *this;
}

sound_sender::~sound_sender() {
	if (m_inputDevice) {
		alcCaptureStop(m_inputDevice);
		alcCaptureCloseDevice(m_inputDevice);
	}
}

void sound_sender::send_sample(const breep::tcp::network& net) noexcept {

	ALCint samplesIn = 0; // How many samples were captured

	// Poll for captured audio
	alcGetIntegerv(m_inputDevice, ALC_CAPTURE_SAMPLES, 1, &samplesIn);

	if (samplesIn > cst::cap_size) {
		// Grab the sound
		alcCaptureSamples(m_inputDevice, buffer.data(), cst::cap_size);
		net.send_object(buffer);
	}
}

