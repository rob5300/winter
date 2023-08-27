#pragma once

#include <utility/utility.hpp>
#include <version/version.hpp>
#include <json/single_include/nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <system_error>
#include <array>
#include <filesystem>

struct KachemakVersion {
	std::string szFileName;
	std::string szDownloadUrl;
	std::size_t lDownloadSize;
	std::size_t lExtractSize;
	std::string szHealUrl;
	std::string szVersion;
	std::string szSignature;
};

class Kachemak : public Version {
public:
	Kachemak(const std::filesystem::path& szInstallPath, const std::filesystem::path& szDataDirectory, const std::string& szSourceUrl = "https://wiki.tf2classic.com/kachemak/");
	KachemakVersion GetVersion(const std::string& version);
	KachemakVersion GetLatestVersion();
private:
	nlohmann::json m_parsedVersion;
	std::filesystem::path m_szTempPath;
};