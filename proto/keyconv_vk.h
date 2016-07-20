#include "keys.pb.h"

namespace keyconv {
	int vk_of_proto(msgs::Keycode keycode);
	msgs::Keycode proto_of_vk(int vk);
}