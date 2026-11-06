// 3Ship's own lines, and OoT lines that the merge rewrites. See HyruleLines.h.
//
// Lines are written in the game's text encoding, as OoT's are once converted (oot.o2r): 0x11 a new line, 0x10 a
// new box, 0x01 red and 0x00 white, 0x19 an event the speaker waits on, 0xBF the end. Literals are split after each
// \x escape, which would otherwise take the hex digits that follow, and built with ""s to keep the 0x00s.

#include <string>

#include "HyruleLines.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
#include "HyruleIds.h"
}

using namespace std::string_literals;

namespace {

CustomMessage::Entry MakeEntry(const std::string& msg, u8 textboxType, u8 textboxYPos) {
    CustomMessage::Entry entry;

    entry.textboxType = textboxType;
    entry.textboxYPos = textboxYPos;
    entry.autoFormat = false;
    entry.msg = msg;
    return entry;
}

// The Happy Mask Salesman's lines, in his textbox (OoT's 0x70xx: type 0, y 48).
const std::string sMaskGift = "These are for you as well:\x11the \x01"s
                              "Goron Mask\x00"s
                              ", the \x01"s
                              "Zora Mask\x00"s
                              "\x11"s
                              "and the \x01"s
                              "Gerudo Mask\x00"s
                              ".\x10Masks are happiest with someone\x11who wears them. Keep them all!\x11"s
                              "Just have faith...\x19\xBF"s;

const std::string sMaskGiftWithHint = "These are for you as well:\x11the \x01"s
                                      "Goron Mask\x00"s
                                      ", the \x01"s
                                      "Zora Mask\x00"s
                                      "\x11"s
                                      "and the \x01"s
                                      "Gerudo Mask\x00"s
                                      ".\x10Masks are happiest with someone\x11who wears them. Keep them all!\x10"s
                                      "When you carry a sword, the\x11\x01"s
                                      "Ocarina of Time\x00"s
                                      " and a song that\x11"s
                                      "calls a horse, come and see me\x11"s
                                      "again. Ho ho ho!\x19\xBF"s;

const std::string sHeroShieldOffer = "Oh ho! A sword, the \x01"s
                                     "Ocarina of\x11Time\x00"s
                                     ", and a song that calls a\x11horse...\x10"s
                                     "Then take this \x01"s
                                     "Hero's Shield\x00"s
                                     ".\x11Where I am going next, a hero\x11"s
                                     "may well be needed.\x19\xBF"s;

const std::string sSalesmanFarewell = "My business in Hyrule is done.\x11I'm off to a faraway land...\x10"s
                                      "If the woods ever lead you astray,\x11perhaps we will meet again.\x11"s
                                      "Ho ho ho!\x19\xBF"s;

const std::string& GetLine(HyruleLineId id) {
    switch (id) {
        case HYRULE_LINE_MASK_GIFT:
            return sMaskGift;
        case HYRULE_LINE_MASK_GIFT_WITH_HINT:
            return sMaskGiftWithHint;
        case HYRULE_LINE_HERO_SHIELD_OFFER:
            return sHeroShieldOffer;
        case HYRULE_LINE_SALESMAN_FAREWELL:
        default:
            return sSalesmanFarewell;
    }
}

// Ingo's race lines, rewritten for docs/EPONA.md E3: the second race is for the ranch, not the horse (type 0, y 0
// for 0x203A, y 48 for 0x203B, as OoT's).
const std::string sIngoRematch = "\x14\x0A"s
                                 "Sh-\x17shoooot!\x18\x10"s
                                 "If the great Ganondorf found out\x11"s
                                 "about this humiliation...\x11"s
                                 "Hey, you!!  How about another\x11"s
                                 "race!  If you win...\x10"s
                                 "I'll give...the ranch back\x11"s
                                 "to Talon!!\x19\xBF"s;

const std::string sIngoLost = "What's up with that horse?!\x11Is that \x01"s
                              "Epona\x00"s
                              "?\x10How did you tame that wild horse\x11right under my nose?!\x10"s
                              "I was going to present that horse\x11to the great Ganondorf... But I bet\x11"s
                              "the ranch on the race and lost!\x11Shooot!\x19\xBF"s;

void RegisterRewrite(u16 textId, const std::string& msg, u8 textboxYPos) {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnOpenText>(
        textId, [msg, textboxYPos](u16* id, bool* loadFromMessageTable) {
            *loadFromMessageTable = false;
            CustomMessage::LoadCustomMessageIntoFont(MakeEntry(msg, 0, textboxYPos));
        });
}

void RegisterHyruleLines() {
    RegisterRewrite(TEXT_OOT(0x203A), sIngoRematch, 0);
    RegisterRewrite(TEXT_OOT(0x203B), sIngoLost, 48);
}

RegisterShipInitFunc sInitFunc(RegisterHyruleLines, {});

} // namespace

extern "C" void Hyrule_ContinueTextbox(PlayState* play, HyruleLineId id) {
    CustomMessage::SetActiveCustomMessage(GetLine(id), MakeEntry(GetLine(id), 0, 48));
    Message_ContinueTextbox(play, CUSTOM_MESSAGE_ID);
}
