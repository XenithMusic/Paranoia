#include <stdint.h>
#include "data.h"
namespace Data {
    uint8_t fallbackFont[] = {
/*$$embed("embed/font.raw")#*/
    };
    const size_t fallbackFontSize = sizeof(fallbackFont);
}