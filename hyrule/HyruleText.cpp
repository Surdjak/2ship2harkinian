// OoT's messages, appended to MM's message table with TEXT_OOT_BASE added to their ids. See hyrule/README.md.

#include <cstdlib>
#include <vector>

#include "HyruleIds.h"
#include <ship/resource/ResourceManager.h>
#include "2s2h/resource/type/TextMM.h"

extern "C" {
#include <message_data_static.h>
}

// mm/2s2h/z_message_OTR.cpp
MessageTableEntry* OTRMessage_LoadTable(const char* filePath, bool isNES);

namespace {

constexpr const char* kOotMessages = "oot/text/nes_message_data_static/ntsc_nes_message_data_static";
constexpr const char* kOotCredits = "oot/text/staff_message_data_static/staff_message_data_static";

// OoT's table also holds the special ids 0xFFFD and 0xFFFF, which are not messages.
bool IsOotMessage(u16 ootTextId) {
    return ootTextId < 0xFFFD;
}

} // namespace

// Returns a table holding MM's messages, then OoT's, ended by id 0xFFFF. Both tables are loaded with
// OTRMessage_LoadTable, which ends them the same way.
extern "C" MessageTableEntry* Hyrule_AddOotMessages(MessageTableEntry* mmTable) {
    MessageTableEntry* ootTable = OTRMessage_LoadTable(kOotMessages, true);
    if (ootTable == nullptr) {
        return mmTable;
    }

    std::vector<MessageTableEntry> merged;
    for (MessageTableEntry* entry = mmTable; entry->textId != 0xFFFF; entry++) {
        merged.push_back(*entry);
    }
    for (MessageTableEntry* entry = ootTable; entry->textId != 0xFFFF; entry++) {
        if (!IsOotMessage(entry->textId)) {
            continue;
        }
        MessageTableEntry oot = *entry;
        oot.textId = TEXT_OOT(entry->textId);
        // The header's next message id (bytes 3-4, see OTRMessage_LoadTable) points into OoT's ids too.
        char* segment = (char*)oot.segment;
        u16 next = ((u8)segment[3] << 8) | (u8)segment[4];
        if (next != 0xFFFF) {
            next = TEXT_OOT(next);
            segment[3] = next >> 8;
            segment[4] = next & 0xFF;
        }
        merged.push_back(oot);
    }
    merged.push_back({});
    merged.back().textId = 0xFFFF;

    MessageTableEntry* table = (MessageTableEntry*)malloc(sizeof(MessageTableEntry) * merged.size());
    std::copy(merged.begin(), merged.end(), table);
    // The entries' text stays in use, only the arrays are replaced.
    free(mmTable);
    free(ootTable);
    return table;
}

// Returns a table holding MM's credits (mmCount entries, as mm/2s2h/z_message_OTR.cpp loads them), then OoT's (its
// ending's staff texts, ids 0x0500-0x05FF) with TEXT_OOT_BASE added to their ids, ended by id 0xFFFF.
extern "C" MessageTableEntry* Hyrule_AddOotCredits(MessageTableEntry* mmTable, size_t mmCount) {
    auto file = std::static_pointer_cast<SOH::TextMM>(
        Ship::Context::GetRawInstance()->GetResourceManager()->LoadResource(kOotCredits));
    if (file == nullptr) {
        return mmTable;
    }

    std::vector<MessageTableEntry> merged;
    for (size_t i = 0; (i < mmCount) && (mmTable[i].textId != 0xFFFF); i++) {
        merged.push_back(mmTable[i]);
    }
    for (const auto& message : file->messages) {
        if (!IsOotMessage(message.id)) {
            continue;
        }
        MessageTableEntry entry = {};
        entry.textId = TEXT_OOT(message.id);
        entry.typePos = (message.textboxType << 4) | message.textboxYPos;
        // The resource stays loaded (the resource manager caches it), so its text can be used in place, as MM's is.
        entry.segment = message.msg.c_str();
        entry.msgSize = message.msg.size();
        merged.push_back(entry);
    }
    merged.push_back({});
    merged.back().textId = 0xFFFF;

    MessageTableEntry* table = (MessageTableEntry*)malloc(sizeof(MessageTableEntry) * merged.size());
    std::copy(merged.begin(), merged.end(), table);
    free(mmTable);
    return table;
}
