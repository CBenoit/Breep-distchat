#ifndef SYSDIST_CHAT_SOUND_SENDER_HPP
#define SYSDIST_CHAT_SOUND_SENDER_HPP

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


#include <breep/network/tcp.hpp>

#include "sound_def.hpp"


class sound_sender {
public:
	explicit sound_sender() noexcept;
	sound_sender(sound_sender&& other) noexcept;
	sound_sender& operator=(sound_sender&& other) noexcept;

	~sound_sender();

	sound_sender(const sound_sender&) = delete;
	sound_sender& operator=(const sound_sender&) = delete;

	void send_sample(const breep::tcp::network& net) noexcept;

private:
	ALCdevice* m_inputDevice;

	sound_buffer_t buffer;     // A buffer to hold captured audio

};


#endif //SYSDIST_CHAT_SOUND_SENDER_HPP
