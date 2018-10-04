#ifndef SYSDIST_CHAT_SOUND_DEF_HPP
#define SYSDIST_CHAT_SOUND_DEF_HPP

#include <AL/alc.h>
#include <breep/util/type_traits.hpp>

namespace cst {
	constexpr ALCuint frequency = 22050;
	constexpr int cap_size = 8192;
}

using sound_buffer_t = std::array<short, 2 * cst::frequency>;

BREEP_DECLARE_TYPE(sound_buffer_t)

#endif //SYSDIST_CHAT_SOUND_DEF_HPP
