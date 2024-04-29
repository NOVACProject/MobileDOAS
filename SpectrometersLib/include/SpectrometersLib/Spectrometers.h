#pragma once

namespace mobiledoas
{

// ListSpectrometerInterfaces lists _all_ SpectrometerInterface:s which can be instantiated with the current build.
// This is the main exported function of this library, and is what makes it possible to list all supported devices.
std::vector<std::unique_ptr<mobiledoas::SpectrometerInterface>> ListSpectrometerInterfaces(mobiledoas::SpectrometerConnectionType connectionType);
}