#ifndef KEYCONV_H
#define KEYCONV_H

#include "keys.pb.h"
#include <Qt>

namespace keyconv
{
    int toQtKey(msgs::Keycode proto_key);
    msgs::Keycode toKeycode(int qt_key);
}

#endif // KEYCONV_H
