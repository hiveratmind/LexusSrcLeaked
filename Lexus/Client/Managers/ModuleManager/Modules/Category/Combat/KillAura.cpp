#include "KillAura.h"
#include "../../Client/Client.h"

bool hasTarget;
bool Stricthits = false;
static size_t hybridIndex = 0;

KillAura::KillAura() : Module(Category::Combat, "KillAura", "Automatically attacks entities") {
    addSetting(new BoolSetting("Strict", "Show target", &Stricthits, false));
    addSetting(new BoolSetting("Render", "Show target", &render, false));
    addSetting(new ColorSetting("Render Color", "Color of render", &renderColor, UIColor(255, 255, 255)));

    addSetting(new SliderSetting<int>("Min CPS", "Minimum clicks per second", &minCPS, 0, 0, 20));
    addSetting(new SliderSetting<int>("Max CPS", "Maximum clicks per second", &maxCPS, 12, 0, 20));

    addSetting(new SliderSetting<float>("Min Reach", "Minimum reach distance", &minReach, 3.f, 0.f, 12.f));
    addSetting(new SliderSetting<float>("Max Reach", "Maximum reach distance", &maxReach, 6.f, 0.f, 12.f));

    addSetting(new EnumSetting("Rotations", "Rotation type", { "None", "Strafe", "Client" }, &rotationMode, 0));
    addSetting(new SliderSetting<int>("Rotation Min Step", "Minimum rotation step", &rotationMinStep, 1, 1, 90));
    addSetting(new SliderSetting<int>("Rotation Max Step", "Maximum rotation step", &rotationMaxStep, 10, 1, 90));

    addSetting(new EnumSetting("Target", "Target mode", { "Single", "Multi", "Hybrid" }, &attackMode, 0));
    addSetting(new BoolSetting("Swing", "Swing arm", &swing, true));
    addSetting(new EnumSetting("Switch", "Weapon switch mode", { "None", "Regular", "Spoof" }, &switchMode, 0));
}

void KillAura::onEnable() {
    targetList.clear();
    lastAttackTime = 0;
    currentRot = { 0.f, 0.f };
    shouldRotate = false;
    hasTarget = false;
    hybridIndex = 0;
}

void KillAura::onDisable() {
    targetList.clear();
    shouldRotate = false;
    hasTarget = false;
    hybridIndex = 0;
}

std::string KillAura::getModeText() {
    return hasTarget ? "Attacking" : "Idle";
}

bool KillAura::sortByDistance(Actor* a, Actor* b) {
    auto player = g_Data.getLocalPlayer();
    if (!player) return false;
    auto pos = player->getPos();
    return a->getPos().dist(pos) < b->getPos().dist(pos);
}

int KillAura::selectBestWeapon(Actor* target) {
    auto player = g_Data.getLocalPlayer();
    if (!player || !target) return 0;

    float bestDamage = 0.f;
    int bestSlot = player->playerInventory ? player->playerInventory->selectedSlot : 0;
    if (!player->playerInventory) return bestSlot;

    for (int i = 0; i < 9; i++) {
        player->playerInventory->selectedSlot = i;
        float dmg = player->calculateAttackDamage(target);
        if (dmg > bestDamage) {
            bestDamage = dmg;
            bestSlot = i;
        }
    }

    return bestSlot;
}

bool KillAura::spoofSwitch(int slot) {
    auto player = g_Data.getLocalPlayer();
    if (!player || !player->playerInventory) return false;

    auto item = player->playerInventory->container ? player->playerInventory->container->getItem(slot) : nullptr;
    if (!item) return false;

    auto packetShared = MinecraftPackets::createPacket(PacketID::MobEquipment);
    if (!packetShared) return false;

    auto* pkt = reinterpret_cast<MobEquipmentPacket*>(packetShared.get());
    pkt->mSlot = slot;
    pkt->mSelectedSlot = slot;
    pkt->mContainerId = 0;
    pkt->mSlotByte = slot;
    pkt->mSelectedSlotByte = slot;
    pkt->mContainerIdByte = 0;

    auto sender = player->level ? player->level->getPacketSender() : nullptr;
    if (!sender) return false;

    sender->send(pkt);
    sender->sendToServer(pkt);
    return true;
}

void KillAura::randomizeSettings() {
    if (minCPS > maxCPS) {
        std::swap(minCPS, maxCPS);
    }
    if (minReach > maxReach) {
        std::swap(minReach, maxReach);
    }
    if (rotationMinStep > rotationMaxStep) {
        std::swap(rotationMinStep, rotationMaxStep);
    }

    currentCPS = minCPS + (maxCPS - minCPS > 0 ? rand() % (maxCPS - minCPS + 1) : 0);
    float reachRange = maxReach - minReach;
    currentReach = minReach + (reachRange > 0.f ? ((float)(rand() % ((int)(reachRange * 100 + 1))) / 100.f) : 0.f);
}

void sendAttackPacket(Actor* target) noexcept {
    auto* localPlayer = g_Data.getLocalPlayer();

    auto attackInvTransac = ItemUseOnActorInventoryTransaction::make_unique();
    attackInvTransac->actorRuntimeId = target->getRuntimeIDComponent()->runtimeId.id;
    attackInvTransac->actionType = ItemUseOnActorInventoryTransaction::ActionType::Attack;
    attackInvTransac->slot = localPlayer->playerInventory->selectedSlot;
    attackInvTransac->itemInHand = NetworkItemStackDescriptor(localPlayer->playerInventory->container->getItem(attackInvTransac->slot));
    attackInvTransac->playerPos = localPlayer->getPos();
    attackInvTransac->clickPos = Vec3<float>{ 0.f, 0.f, 0.f };
    InventoryTransactionPacket itp(std::move(attackInvTransac));
    localPlayer->level->getPacketSender()->send(&itp);
    g_Data.getLocalPlayer()->swing();
}

void KillAura::performAttack(Actor* target) {
    auto player = g_Data.getLocalPlayer();
    if (!player || !target) return;
    if (!g_Data.canUseMoveKeys()) return;
    if (!player->gameMode) return;

    if (Stricthits) {
        if (player->getlevel()->getHitResult()->type != HitResultType::ENTITY) return;
    }

    player->getlevel()->getHitResult()->type = HitResultType::ENTITY;

    g_Data.getLocalPlayer()->getgameMode()->attack(target);
    if (swing) {
        g_Data.getLocalPlayer()->swing();
    }
}

float normalizeAngle(float angle) {
    while (angle > 180.f) angle -= 360.f;
    while (angle < -180.f) angle += 360.f;
    return angle;
}

float shortestAngleDiff(float current, float target) {
    float diff = normalizeAngle(target - current);
    return diff;
}

float lerpFloat(float start, float end, float percent) {
    return start + (end - start) * percent;
}

float lerpAngle(float current, float target, float percent) {
    float diff = shortestAngleDiff(current, target);
    return normalizeAngle(current + diff * percent);
}

void KillAura::OnTick() {
    if (minCPS > maxCPS) {
        maxCPS = minCPS;
    }
    if (minReach > maxReach) {
        maxReach = minReach;
    }

    auto player = g_Data.getLocalPlayer();
    if (!player || !player->level) {
        targetList.clear();
        hasTarget = false;
        return;
    }

    if (!g_Data.canUseMoveKeys()) {
        targetList.clear();
        hasTarget = false;
        return;
    }

    targetList.clear();
    randomizeSettings();

    for (auto* ent : ActorUtils::getActorList(true)) {
        if (!ent) continue;
        if (!ent->getActorTypeComponent()) continue;
        if (!ent->stateVector) continue;
        if (!ent->isAlive()) continue;
        if (!TargetUtil::ValidCheck(ent, false)) continue;
        if (player->getPos().dist(ent->getPos()) <= maxReach) {
            targetList.push_back(ent);
        }
    }

    std::sort(targetList.begin(), targetList.end(), sortByDistance);
    hasTarget = !targetList.empty();

    if (hasTarget) {
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        float interval = currentCPS > 0 ? (1000.f / currentCPS) : 1000.f;

        if (now - lastAttackTime >= interval) {
            if (attackMode == 0) {
                for (auto& target : targetList) {
                    if (!target) continue;
                    if (player->getPos().dist(target->getPos()) <= currentReach) {
                        int oldSlot = player->playerInventory ? player->playerInventory->selectedSlot : 0;
                        int bestSlot = oldSlot;
                        if (switchMode == 1) {
                            bestSlot = selectBestWeapon(target);
                        }
                        else if (switchMode == 2) {
                            spoofSwitch(selectBestWeapon(target));
                        }
                        if (switchMode == 1 && player->playerInventory) {
                            player->playerInventory->selectedSlot = bestSlot;
                        }
                        performAttack(target);
                        break;
                    }
                }
            }
            else if (attackMode == 1) {
                for (auto& target : targetList) {
                    if (!target) continue;
                    if (player->getPos().dist(target->getPos()) <= currentReach) {
                        int oldSlot = player->playerInventory ? player->playerInventory->selectedSlot : 0;
                        int bestSlot = oldSlot;
                        if (switchMode == 1) {
                            bestSlot = selectBestWeapon(target);
                        }
                        else if (switchMode == 2) {
                            spoofSwitch(selectBestWeapon(target));
                        }
                        if (switchMode == 1 && player->playerInventory) {
                            player->playerInventory->selectedSlot = bestSlot;
                        }
                        performAttack(target);
                    }
                }
            }
            else if (attackMode == 2) {
                if (!targetList.empty()) {
                    hybridIndex %= targetList.size();
                    Actor* target = targetList[hybridIndex];
                    if (target && player->getPos().dist(target->getPos()) <= currentReach) {
                        int oldSlot = player->playerInventory ? player->playerInventory->selectedSlot : 0;
                        int bestSlot = oldSlot;
                        if (switchMode == 1) {
                            bestSlot = selectBestWeapon(target);
                        }
                        else if (switchMode == 2) {
                            spoofSwitch(selectBestWeapon(target));
                        }
                        if (switchMode == 1 && player->playerInventory) {
                            player->playerInventory->selectedSlot = bestSlot;
                        }
                        performAttack(target);
                        hybridIndex++;
                    }
                    else {
                        hybridIndex++;
                    }
                }
            }
            lastAttackTime = now;
        }
    }

    shouldRotate = rotationMode != 0;
}

void KillAura::RotationUpdateHook(LocalPlayer* player) {
    if (!shouldRotate || !player || !player->rotation || !player->getActorHeadRotationComponent()) return;
    if (targetList.empty() || !hasTarget) return;

    Actor* target = nullptr;

    if (attackMode == 2) { // Hybrid
        if (!targetList.empty()) {
            hybridIndex %= targetList.size();
            target = targetList[hybridIndex];
        }
    }
    else if (attackMode == 0) { // Single
        target = targetList[0];
    }
    else if (attackMode == 1) { // Multi
        target = targetList[0];
    }

    if (!target) return;

    Vec2<float> targetAngle = player->getEyePos().CalcAngle(target->getEyePos());

    int minStep = std::min(rotationMinStep, rotationMaxStep);
    int maxStep = std::max(rotationMinStep, rotationMaxStep);

    float yawStep = static_cast<float>(minStep + (rand() % (maxStep - minStep + 1)));
    float pitchStep = static_cast<float>(minStep + (rand() % (maxStep - minStep + 1)));

    float yawDiff = shortestAngleDiff(currentRot.y, targetAngle.y);
    float pitchDiff = shortestAngleDiff(currentRot.x, targetAngle.x);

    if (std::abs(yawDiff) > yawStep)
        currentRot.y += (yawDiff > 0 ? yawStep : -yawStep);
    else
        currentRot.y = targetAngle.y;

    if (std::abs(pitchDiff) > pitchStep)
        currentRot.x += (pitchDiff > 0 ? pitchStep : -pitchStep);
    else
        currentRot.x = targetAngle.x;

    currentRot.x = std::clamp(currentRot.x, -90.f, 90.f);

    if (rotationMode != 2) {
        player->rotation->presentRot.y = currentRot.y;
    }

    if (rotationMode == 2) {
        float bodyYaw = player->getMobBodyRotationComponent()->bodyYaw;
        float diff = currentRot.y - bodyYaw;
        while (diff > 180.f) diff -= 360.f;
        while (diff < -180.f) diff += 360.f;

        float absDiff = std::abs(diff);
        if (absDiff >= 25.f) {
            float targetYaw = currentRot.y - (diff > 0 ? 18.f : -18.f);
            float yawDiff = targetYaw - bodyYaw;
            while (yawDiff > 180.f) yawDiff -= 360.f;
            while (yawDiff < -180.f) yawDiff += 360.f;
            targetYaw = bodyYaw + yawDiff;

            player->getMobBodyRotationComponent()->bodyYaw = lerpAngle(bodyYaw, targetYaw, 0.15f);
        }
    }

    player->rotation->presentRot.x = currentRot.x;
    player->getActorHeadRotationComponent()->headYaw = currentRot.y;
}

void KillAura::SentPacketHook(Packet* packet, bool& cancel) {
    if (!shouldRotate || rotationMode == 0 || !packet) return;
    if (packet->getId() == PacketID::PlayerAuthInput) {
        auto* input = reinterpret_cast<PlayerAuthInputPacket*>(packet);
        if (!hasTarget) return;
        Actor* target = targetList[0];
        if (!target) return;
        input->rotation = currentRot;
        input->headYaw = currentRot.y;
    }
}

void KillAura::LevelRenderHook() {
    if (!render || !g_Data.clientInstance) return;

    auto* levelRenderer = g_Data.clientInstance->getLevelRenderer();
    if (!levelRenderer || !levelRenderer->levelRendererPlayer) return;

    Vec3<float> origin = levelRenderer->levelRendererPlayer->realOrigin;

    auto* localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer || targetList.empty()) return;

    Actor* target = targetList[0];
    if (!target || !target->aabbShape) return;

    auto& aabb = target->aabbShape->aabb;
    Vec3<float> lower = aabb.lower;
    Vec3<float> upper = aabb.upper;

    float widthX = upper.x - lower.x;
    float widthZ = upper.z - lower.z;
    float height = upper.y - lower.y;

    Vec3<float> targetPos = target->getEyePos();

    auto* actorTypeComp = target->getActorTypeComponent();
    if (!actorTypeComp) return;

    if (actorTypeComp->id != 319)
        targetPos.y = upper.y;

    static float anim = 0.f;
    anim += RenderUtils::deltaTime * 62.f;
    float animYOffset = 1.f + sinf((anim / 60.f) * PI);

    targetPos.y = targetPos.y - 1.6f + animYOffset;

    constexpr float degToRad = PI / 180.f;
    std::array<Vec3<float>, 360> posList;

    float radiusX = widthX * 1.1f;
    float radiusZ = widthZ * 1.1f;

    for (int i = 0; i < 360; ++i) {
        float angle = (i + 90.f) * degToRad;
        float x = targetPos.x + cosf(angle) * radiusX;
        float y = targetPos.y;
        float z = targetPos.z + sinf(angle) * radiusZ;
        posList[i] = { x - origin.x, y - origin.y, z - origin.z };
    }

    RenderUtils::SetColor(UIColor(renderColor.r, renderColor.g, renderColor.b, 15));
    RenderUtils::tessellator->begin(VertextFormat::QUAD);

    static float smoothYOffset = 1.f;
    smoothYOffset = Math::lerp(smoothYOffset, animYOffset, RenderUtils::deltaTime * 10.f);
    float startY = targetPos.y - animYOffset + smoothYOffset;

    int steps = 15;
    int drawn = 0;

    for (float y = targetPos.y; (y < startY - 0.001f || y > startY + 0.001f) && drawn < steps; y = Math::lerp(y, startY, 0.12f), ++drawn) {
        for (int i = 0; i < 360; ++i) {
            Vec3<float> p1 = posList[i];
            Vec3<float> p2 = posList[(i + 1) % 360];
            Vec3<float> p3 = { p2.x, y - origin.y, p2.z };
            Vec3<float> p4 = { p1.x, y - origin.y, p1.z };

            RenderUtils::tessellator->vertex(p1.x, p1.y, p1.z);
            RenderUtils::tessellator->vertex(p2.x, p2.y, p2.z);
            RenderUtils::tessellator->vertex(p3.x, p3.y, p3.z);
            RenderUtils::tessellator->vertex(p4.x, p4.y, p4.z);

            RenderUtils::tessellator->vertex(p4.x, p4.y, p4.z);
            RenderUtils::tessellator->vertex(p3.x, p3.y, p3.z);
            RenderUtils::tessellator->vertex(p2.x, p2.y, p2.z);
            RenderUtils::tessellator->vertex(p1.x, p1.y, p1.z);
        }
    }

    MeshHelpers::renderMeshImmediately(RenderUtils::screenCtx, RenderUtils::tessellator, RenderUtils::blendMaterial);

    RenderUtils::SetColor(renderColor);
    RenderUtils::tessellator->begin(VertextFormat::LINE_LIST);

    for (int i = 0; i < 360; ++i) {
        const auto& p1 = posList[i];
        const auto& p2 = posList[(i + 1) % 360];
        RenderUtils::tessellator->vertex(p1.x, p1.y, p1.z);
        RenderUtils::tessellator->vertex(p2.x, p2.y, p2.z);
    }

    MeshHelpers::renderMeshImmediately(RenderUtils::screenCtx, RenderUtils::tessellator, RenderUtils::blendMaterial);
}
