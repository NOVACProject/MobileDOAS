#pragma once

// This file is an attempt to collect the previously scattered configuration file
//  handling into one location.

namespace Configuration
{
class CMobileConfiguration;

class ConfigurationFile
{
public:

    /** Writes the provided configuration to the default location.
        @throws a subclass to std::exception if someting goes wrong. */
    static void Write(CMobileConfiguration& configuration);

};

}
