#include "Harness.h"

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>

namespace {

constexpr int kInputVersion = 1;

struct HarnessContext {
    bool initialized = false;
    std::string game;
    uint32_t seed = 0;
    uint64_t frame = 0;

    FILE* record = nullptr;
    std::vector<HarnessPad> replay;
    bool replaying = false;
    FILE* stateLog = nullptr;

    // Last flag block hashes written, so full flag data is only logged when it changes.
    std::map<std::string, uint64_t> lastFlagHashes;

    // Puppet player states keyed by (scene, scene frame).
    std::map<std::pair<int32_t, uint32_t>, HarnessPlayer> puppet;
};

HarnessContext sCtx;

uint64_t Fnv1a(const void* data, size_t size, uint64_t hash = 0xCBF29CE484222325ULL) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; i++) {
        hash ^= bytes[i];
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

uint64_t HashString(const std::string& s) {
    return Fnv1a(s.data(), s.size());
}

std::string Hex64(uint64_t value) {
    char buf[17];
    snprintf(buf, sizeof(buf), "%016" PRIx64, value);
    return buf;
}

std::string HexBytes(const std::vector<uint8_t>& data) {
    static const char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(data.size() * 2);
    for (uint8_t b : data) {
        out += digits[b >> 4];
        out += digits[b & 0xF];
    }
    return out;
}

nlohmann::json PlayerToJson(const HarnessPlayer& p) {
    return {
        { "pos", { p.pos[0], p.pos[1], p.pos[2] } },
        { "rot", { p.rot[0], p.rot[1], p.rot[2] } },
        { "speedXZ", p.speedXZ },
        { "yaw", p.yaw },
        { "stateFlags", { p.stateFlags[0], p.stateFlags[1], p.stateFlags[2] } },
        { "form", p.form },
    };
}

bool PlayerFromJson(const nlohmann::json& j, HarnessPlayer* p) {
    try {
        for (int i = 0; i < 3; i++) {
            p->pos[i] = j.at("pos").at(i).get<float>();
            p->rot[i] = j.at("rot").at(i).get<int16_t>();
            p->stateFlags[i] = j.at("stateFlags").at(i).get<uint32_t>();
        }
        p->speedXZ = j.at("speedXZ").get<float>();
        p->yaw = j.at("yaw").get<int16_t>();
        p->form = j.at("form").get<int32_t>();
        return true;
    } catch (const nlohmann::json::exception&) {
        return false;
    }
}

const char* Env(const char* name) {
    const char* value = std::getenv(name);
    return (value != nullptr && value[0] != '\0') ? value : nullptr;
}

void LoadReplay(const char* path) {
    std::ifstream in(path);
    if (!in) {
        fprintf(stderr, "[harness] cannot open replay file %s\n", path);
        std::exit(1);
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        HarnessPad pad = {};
        unsigned long long frame;
        unsigned button;
        int sx, sy, rx, ry;
        if (sscanf(line.c_str(), "%llu %x %d %d %d %d %f %f", &frame, &button, &sx, &sy, &rx, &ry, &pad.gyroX,
                   &pad.gyroY) != 8) {
            fprintf(stderr, "[harness] bad replay line: %s\n", line.c_str());
            std::exit(1);
        }
        if (frame != sCtx.replay.size()) {
            fprintf(stderr, "[harness] replay frames must be consecutive from 0 (got %llu)\n", frame);
            std::exit(1);
        }
        pad.button = button;
        pad.stickX = (int8_t)sx;
        pad.stickY = (int8_t)sy;
        pad.rightStickX = (int8_t)rx;
        pad.rightStickY = (int8_t)ry;
        sCtx.replay.push_back(pad);
    }
    sCtx.replaying = true;
    fprintf(stderr, "[harness] replaying %zu frames from %s\n", sCtx.replay.size(), path);
}

void LoadPuppet(const char* path) {
    std::ifstream in(path);
    if (!in) {
        fprintf(stderr, "[harness] cannot open puppet log %s\n", path);
        std::exit(1);
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        nlohmann::json j = nlohmann::json::parse(line, nullptr, false);
        if (j.is_discarded() || !j.contains("player")) {
            continue;
        }
        HarnessPlayer p;
        if (PlayerFromJson(j["player"], &p)) {
            sCtx.puppet[{ j.value("scene", -1), j.value("sf", 0u) }] = p;
        }
    }
    fprintf(stderr, "[harness] puppet log %s has %zu player frames\n", path, sCtx.puppet.size());
}

} // namespace

extern "C" void Harness_Init(const char* game) {
    if (sCtx.initialized) {
        return;
    }
    sCtx.initialized = true;
    sCtx.game = game;

    if (const char* seed = Env("HARNESS_SEED")) {
        sCtx.seed = (uint32_t)strtoul(seed, nullptr, 0);
    }
    if (const char* path = Env("HARNESS_REPLAY")) {
        LoadReplay(path);
    }
    if (const char* path = Env("HARNESS_RECORD")) {
        if (sCtx.replaying) {
            fprintf(stderr, "[harness] HARNESS_RECORD and HARNESS_REPLAY are exclusive\n");
            std::exit(1);
        }
        sCtx.record = fopen(path, "w");
        if (sCtx.record == nullptr) {
            fprintf(stderr, "[harness] cannot write %s\n", path);
            std::exit(1);
        }
        fprintf(sCtx.record, "# harness-inputs %d game=%s seed=%u\n", kInputVersion, game, sCtx.seed);
        fprintf(sCtx.record, "# frame button stickX stickY rightStickX rightStickY gyroX gyroY\n");
    }
    if (const char* path = Env("HARNESS_STATELOG")) {
        sCtx.stateLog = fopen(path, "w");
        if (sCtx.stateLog == nullptr) {
            fprintf(stderr, "[harness] cannot write %s\n", path);
            std::exit(1);
        }
        fprintf(sCtx.stateLog, "# harness-state 1 game=%s seed=%u\n", game, sCtx.seed);
    }
    if (const char* path = Env("HARNESS_PUPPET")) {
        LoadPuppet(path);
    }
    if (Harness_IsActive()) {
        fprintf(stderr, "[harness] active for %s, seed %u\n", game, sCtx.seed);
    }
}

extern "C" void Harness_Shutdown(void) {
    if (sCtx.record != nullptr) {
        fclose(sCtx.record);
        sCtx.record = nullptr;
    }
    if (sCtx.stateLog != nullptr) {
        fclose(sCtx.stateLog);
        sCtx.stateLog = nullptr;
    }
}

extern "C" bool Harness_IsActive(void) {
    return sCtx.record != nullptr || sCtx.replaying || sCtx.stateLog != nullptr || !sCtx.puppet.empty();
}

extern "C" uint32_t Harness_Seed(void) {
    return sCtx.seed;
}

extern "C" uint64_t Harness_Frame(void) {
    return sCtx.frame;
}

extern "C" bool Harness_ProcessPad(HarnessPad* pad) {
    uint64_t frame = sCtx.frame++;

    if (sCtx.replaying) {
        if (frame >= sCtx.replay.size()) {
            return false;
        }
        *pad = sCtx.replay[frame];
    }
    if (sCtx.record != nullptr) {
        fprintf(sCtx.record, "%" PRIu64 " %x %d %d %d %d %g %g\n", frame, pad->button, pad->stickX, pad->stickY,
                pad->rightStickX, pad->rightStickY, pad->gyroX, pad->gyroY);
        // Flushed every frame so a crash keeps everything recorded up to it.
        fflush(sCtx.record);
    }
    return true;
}

void Harness_LogState(const HarnessFrameState& state) {
    if (sCtx.stateLog == nullptr) {
        return;
    }

    nlohmann::json actors = nlohmann::json::array();
    for (const HarnessActor& a : state.actors) {
        actors.push_back({
            { "cat", a.category },
            { "id", a.id },
            { "params", a.params },
            { "room", a.room },
            { "flags", a.flags },
            { "pos", { a.pos[0], a.pos[1], a.pos[2] } },
            { "rot", { a.rot[0], a.rot[1], a.rot[2] } },
        });
    }

    // Flag blocks are hashed every frame but written out only when they change.
    nlohmann::json flagHashes = nlohmann::json::object();
    nlohmann::json flagData = nlohmann::json::object();
    uint64_t flagsHash = Fnv1a(nullptr, 0);
    for (const HarnessFlagBlock& block : state.flags) {
        uint64_t h = Fnv1a(block.data.data(), block.data.size());
        flagHashes[block.name] = Hex64(h);
        flagsHash = Fnv1a(&h, sizeof(h), Fnv1a(block.name.data(), block.name.size(), flagsHash));
        auto it = sCtx.lastFlagHashes.find(block.name);
        if (it == sCtx.lastFlagHashes.end() || it->second != h) {
            flagData[block.name] = HexBytes(block.data);
            sCtx.lastFlagHashes[block.name] = h;
        }
    }

    // The pad for this logic frame has already been processed, which advanced the counter.
    nlohmann::json line = {
        { "f", sCtx.frame == 0 ? 0 : sCtx.frame - 1 },
        { "gs", state.gameState },
        { "scene", state.scene },
        { "room", state.room },
        { "sf", state.sceneFrame },
        { "rng", state.rng },
    };
    if (state.hasPlayer) {
        line["player"] = PlayerToJson(state.player);
    }
    line["actors"] = actors;
    line["flagHashes"] = flagHashes;
    if (!flagData.empty()) {
        line["flags"] = flagData;
    }

    std::string playerDump = state.hasPlayer ? line["player"].dump() : "";
    std::string actorsDump = actors.dump();
    uint64_t hPlayer = HashString(playerDump);
    uint64_t hActors = HashString(actorsDump);
    uint64_t hAll = HashString(state.gameState + "|" + std::to_string(state.scene) + "|" +
                               std::to_string(state.room) + "|" + std::to_string(state.rng) + "|" + playerDump +
                               "|" + actorsDump + "|" + Hex64(flagsHash));
    line["h"] = {
        { "all", Hex64(hAll) },
        { "player", Hex64(hPlayer) },
        { "actors", Hex64(hActors) },
        { "flags", Hex64(flagsHash) },
    };

    fputs(line.dump().c_str(), sCtx.stateLog);
    fputc('\n', sCtx.stateLog);
    fflush(sCtx.stateLog);
}

bool Harness_PuppetActive() {
    return !sCtx.puppet.empty();
}

bool Harness_PuppetPlayer(int32_t scene, uint32_t sceneFrame, HarnessPlayer* out) {
    auto it = sCtx.puppet.find({ scene, sceneFrame });
    if (it == sCtx.puppet.end()) {
        return false;
    }
    *out = it->second;
    return true;
}
