/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * August 7, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _RCP_WRAPPER_H
#define _RCP_WRAPPER_H

#include <fstream>
#include "stringHelper.h"
#include "ipackageWrapper.h"
#include "sharedDefs.h"
#include "filesystemHelper.h"

class RCPWrapper final : public IPackageWrapper
{
    public:
        static constexpr auto INFO_PLIST_PATH { "Contents/Info.plist" };

        explicit RCPWrapper(const PackageContext& ctx)
            : m_architecture{UNKNOWN_VALUE}
            , m_format{"rcp"}
            , m_vendor{UNKNOWN_VALUE}
        {
            m_version = UNKNOWN_VALUE;
            m_installTime = UNKNOWN_VALUE;
            m_groups = UNKNOWN_VALUE;
            m_description = UNKNOWN_VALUE;
            m_size = 0;
            m_priority = UNKNOWN_VALUE;
            m_multiarch = UNKNOWN_VALUE;

            std::string pathInstallPlistFile { ctx.filePath + "/" + ctx.package + ".plist" };
            getPlistData(pathInstallPlistFile);

            if(m_installPrefixPath.empty())
            {
                m_installPrefixPath = "/";
            }

            std::string pathPlistFile;
            std::string pathBomFile { ctx.filePath + "/" + ctx.package + ".bom" };
            if(Utils::existsRegular(pathBomFile))
            {
                getBomData(pathBomFile);
                
                std::string infoPlistEndingApp { std::string(".app/") + INFO_PLIST_PATH };
                std::string infoPlistEndingService { std::string(".service/") + INFO_PLIST_PATH };
                size_t numSubdirectoriesMin = (size_t)-1;

                for ( const auto& bomPath : m_bomPaths)
                {
                    if(Utils::endsWith(bomPath, infoPlistEndingApp) || Utils::endsWith(bomPath, infoPlistEndingService))
                    {
                        size_t numSubdirectoriesCurrent = Utils::split(bomPath, '/').size();
                        if(numSubdirectoriesCurrent < numSubdirectoriesMin)
                        {
                            numSubdirectoriesMin = numSubdirectoriesCurrent;
                            pathPlistFile = bomPath;
                        }
                    }
                }
            }

            if(!pathPlistFile.empty() && Utils::existsRegular(pathPlistFile))
            {
                getPlistData(pathPlistFile);
            }
        }

        ~RCPWrapper() = default;

        std::string name() const override
        {
            return m_name;
        }
        std::string version() const override
        {
            return m_version;
        }
        std::string groups() const override
        {
            return m_groups;
        }
        std::string description() const override
        {
            return m_description;
        }
        std::string architecture() const override
        {
            return m_architecture;
        }
        std::string format() const override
        {
            return m_format;
        }
        std::string osPatch() const override
        {
            return m_osPatch;
        }
        std::string source() const override
        {
            return m_source;
        }
        std::string location() const override
        {
            return m_location;
        }
        std::string vendor() const override
        {
            return m_vendor;
        }

        std::string priority() const override
        {
            return m_priority;
        }

        int size() const override
        {
            return m_size;
        }

        std::string install_time() const override
        {
            return m_installTime;
        }

        std::string multiarch() const override
        {
            return m_multiarch;
        }

    private:
        static constexpr auto PLIST_BINARY_HEADER { "bplist00" };
        static constexpr auto UTILITIES_FOLDER { "/Utilities" };

        void getBomData(const std::string& filePath)
        {
            m_bomPaths.clear();
        }

        void getPlistData(const std::string& filePath)
        {
            const auto isBinaryFnc
            {
                [&filePath]()
                {
                    // If first bytes are "bplist00" it's a binary plist file
                    std::array<char, (sizeof(PLIST_BINARY_HEADER) - 1)> headerBuffer;
                    std::ifstream ifs {filePath, std::ios::binary};
                    ifs.read(headerBuffer.data(), sizeof(headerBuffer));
                    return !std::memcmp(headerBuffer.data(), PLIST_BINARY_HEADER, sizeof(PLIST_BINARY_HEADER) - 1);
                }
            };
            const auto isBinary { isBinaryFnc() };
            constexpr auto BUNDLEID_PATTERN{R"(^[^.]+\.([^.]+).*$)"};
            static std::regex bundleIdRegex{BUNDLEID_PATTERN};

            static const auto getValueFnc
            {
                [](const std::string & val)
                {
                    const auto start{val.find(">")};
                    const auto end{val.rfind("<")};
                    return val.substr(start + 1, end - start - 1);
                }
            };

            const auto getDataFnc
            {
                [this, &filePath](std::istream & data)
                {
                    std::string line;
                    std::string bundleShortVersionString;
                    std::string bundleVersion;

                    m_location = filePath;
                    m_source = (filePath.find(UTILITIES_FOLDER) != std::string::npos)? "utilities" : "applications";

                    while (std::getline(data, line))
                    {
                        line = Utils::trim(line, " \t");

                        if (line == "<key>CFBundleName</key>" &&
                            std::getline(data, line))
                        {
                            m_name = getValueFnc(line);
                        }
                        else if ((line == "<key>CFBundleShortVersionString</key>" ||
                                  line == "<key>PackageVersion</key>") &&
                                 std::getline(data, line))
                        {
                            bundleShortVersionString = getValueFnc(line);
                        }
                        else if (line == "<key>CFBundleVersion</key>" &&
                                 std::getline(data, line))
                        {
                            bundleVersion = getValueFnc(line);
                        }
                        else if (line == "<key>LSApplicationCategoryType</key>" &&
                                 std::getline(data, line))
                        {
                            m_groups = getValueFnc(line);
                        }
                        else if ((line == "<key>CFBundleIdentifier</key>" ||
                                  line == "<key>PackageIdentifier</key>") &&
                                 std::getline(data, line))
                        {
                            m_description = getValueFnc(line);

                            std::string vendor;

                            if (Utils::findRegexInString(m_description, vendor, bundleIdRegex, 1))
                            {
                                m_vendor = vendor;
                            }
                        }
                        else if (line == "<key>InstallPrefixPath</key>" &&
                                 std::getline(data, line))
                        {
                            m_installPrefixPath = getValueFnc(line);
                        }
                    }

                    if (m_name.empty() && !m_description.empty())
                    {
                        m_name = m_description;
                    }

                    if (!bundleShortVersionString.empty() && Utils::startsWith(bundleVersion, bundleShortVersionString))
                    {
                        m_version = bundleVersion;
                    }
                    else
                    {
                        m_version = bundleShortVersionString;
                    }
                }
            };

            if (isBinary)
            {
                auto xmlContent { binaryToXML(filePath) };
                getDataFnc(xmlContent);
            }
            else
            {
                std::fstream file { filePath, std::ios_base::in };

                if (file.is_open())
                {
                    getDataFnc(file);
                }
            }
        }

        std::stringstream binaryToXML(const std::string& filePath)
        {
            std::string xmlContent;
            plist_t rootNode { nullptr };
            const auto binaryContent { Utils::getBinaryContent(filePath) };

            // plist C++ APIs calls - to be used when Makefile and external are updated.
            // const auto dataFromBin { PList::Structure::FromBin(binaryContent) };
            // const auto xmlContent { dataFromBin->ToXml() };

            // Content binary file to plist representation
            plist_from_bin(binaryContent.data(), binaryContent.size(), &rootNode);

            if (nullptr != rootNode)
            {
                char* xml { nullptr };
                uint32_t length { 0 };
                // plist binary representation to XML
                plist_to_xml(rootNode, &xml, &length);

                if (nullptr != xml)
                {
                    xmlContent.assign(xml, xml + length);
                    plist_to_xml_free(xml);
                    plist_free(rootNode);
                }
            }

            return std::stringstream{xmlContent};
        }

        std::string m_name;
        std::string m_version;
        std::string m_groups;
        std::string m_description;
        std::string m_architecture;
        const std::string m_format;
        std::string m_osPatch;
        std::string m_source;
        std::string m_location;
        std::string m_multiarch;
        std::string m_priority;
        int m_size;
        std::string m_vendor;
        std::string m_installTime;
        std::string m_installPrefixPath;
        std::deque<std::string> m_bomPaths;
};

#endif //_RCP_WRAPPER_H
