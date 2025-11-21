#include "Ambient.h"

const std::string Ambient::IMAGE_URL = "https://raw.githubusercontent.com/doodlejonfreeman-ctrl/resources/a9ead04a3044cad4f853135e41ec302c8481f41e/pink-star-download-png-701751694532302rtnyawdolh-removebg-preview.png";

Ambient::Ambient() : Module(Category::Render, "Ambient", "Ambient random images around you") {
    spawnRadius = 5.f;
    spawnInterval = 0.1f;
    timer = 0.f;
}

void Ambient::OnTick() {
    timer += 0.05f;
    if (timer >= spawnInterval) {
        timer = 0.f;
        LocalPlayer* lp = g_Data.getLocalPlayer();
        if (!lp) return;
        Vec3 basePos = lp->getPos();
        Letter letter;
        letter.lifetime = 5.f;
        letter.pos = Vec3(
            basePos.x + ((rand() / (float)RAND_MAX) * 2.f - 1.f) * spawnRadius,
            basePos.y + ((rand() / (float)RAND_MAX) * 2.f - 1.f) * spawnRadius,
            basePos.z + ((rand() / (float)RAND_MAX) * 2.f - 1.f) * spawnRadius
        );
        letters.push_back(letter);
    }

    for (size_t i = 0; i < letters.size();) {
        letters[i].lifetime -= 0.05f;
        if (letters[i].lifetime <= 0.f) {
            letters.erase(letters.begin() + i);
        }
        else {
            i++;
        }
    }
}

void Ambient::InterfaceUtilsHook() {
    for (const Letter& letter : letters) {
        Vec2<float> screenSize = g_Data.clientInstance->guiData->windowSizeReal;
        Vec2<float> screenPos = { screenSize.x / 2, screenSize.y / 2 };
        if (RenderUtils::WorldToScreen(letter.pos, screenPos)) {
            float size = 20.f;
           // InterfaceUtils::drawImageFromLink(IMAGE_URL, screenPos, size, size);
        }
    }
}

