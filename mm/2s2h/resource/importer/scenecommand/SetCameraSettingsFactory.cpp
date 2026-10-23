#include "2s2h/resource/importer/scenecommand/SetCameraSettingsFactory.h"
#include "2s2h/resource/type/scenecommand/SetCameraSettings.h"

namespace SOH {
std::shared_ptr<Ship::IResource>
SetCameraSettingsFactory::ReadResource(std::shared_ptr<Ship::ResourceInitData> initData,
                                       std::shared_ptr<Ship::BinaryReader> reader) {
    auto setCameraSettings = std::make_shared<SetCameraSettings>(initData);

    ReadCommandId(setCameraSettings, reader);

    // 3Ship [Hyrule] MM's 0x19 has no payload; OoT scenes (under oot/) keep OoT's camera type and world map area.
    if (initData->Path.rfind("oot/", 0) == 0) {
        setCameraSettings->settings.cameraMovement = reader->ReadInt8();
        setCameraSettings->settings.worldMapArea = reader->ReadInt32();
    }

    return setCameraSettings;
}
} // namespace SOH
