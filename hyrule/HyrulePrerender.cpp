// OoT's prerendered room backgrounds. See hyrule/README.md.

#include "HyrulePrerender.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <ship/Context.h>
#include <stb_image.h>

#include "2s2h/resource/type/Background.h"

namespace {

// Decoded backgrounds by resource path, kept for the session: there are few, and rooms are revisited.
std::unordered_map<std::string, std::vector<u8>> sDecoded;

} // namespace

// OoT stores these backgrounds as 320x240 JPEGs and decodes them into the room's buffer when the room
// loads; SoH decodes them when drawn (Room_DrawBackground2D). Same conversion as SoH's.
extern "C" void* Hyrule_GetPrerenderImage(const char* path) {
    auto found = sDecoded.find(path);
    if (found != sDecoded.end()) {
        return found->second.data();
    }

    auto background = std::dynamic_pointer_cast<SOH::Background>(
        Ship::Context::GetRawInstance()->GetResourceManager()->LoadResource(path));
    if (background == nullptr) {
        return nullptr;
    }
    int width;
    int height;
    int components;
    u8* pixels = stbi_load_from_memory(background->Data.data(), (int)background->Data.size(), &width, &height,
                                       &components, STBI_rgb_alpha);
    if (pixels == nullptr) {
        return nullptr;
    }

    // RGBA5551, big-endian like the N64 texture formats.
    std::vector<u8>& image = sDecoded[path];
    image.resize(width * height * 2);
    for (int i = 0; i < width * height; i++) {
        const u8* pixel = &pixels[i * 4];
        u16 color = ((pixel[0] / 8) << 11) | ((pixel[1] / 8) << 6) | ((pixel[2] / 8) << 1) | (pixel[3] != 0);
        image[i * 2] = color >> 8;
        image[i * 2 + 1] = color & 0xFF;
    }
    stbi_image_free(pixels);
    return image.data();
}
