#include "fremont.hpp"

int fremont::ExecWithParam(const std::vector<std::string>& params) {
  std::string param_str;
  for (const auto& i : params) {
    param_str += i + " ";
  }

  A_printf("%s\n", param_str.c_str());
  return system(param_str.c_str());
}

/*
description:
  recursively delete contents of directory
res:
  0: success
  1: input path doesn't exist
  2: input path isn't a directory
*/
int fremont::DeleteDirectoryContent(const std::filesystem::path& dir) {
  if (!std::filesystem::exists(dir)) {
    return 1;
  }
  if (!std::filesystem::is_directory(dir)) {
    return 2;
  }
  for (const auto& entry : std::filesystem::directory_iterator(dir)) {
    if (std::filesystem::is_directory(entry)) {
      DeleteDirectoryContent(entry);
      std::filesystem::remove(entry);
    } else {
      std::filesystem::remove(entry);
    }
  }
  return 0;
}

/*
desc:
        extract .zip file
returns:
        0: success
*/

int fremont::ExtractZip(const std::string& szInputFile, const std::string& szOutputFile) {
  zip_extract(szInputFile.c_str(), szOutputFile.c_str(), nullptr, nullptr);
  return 0;
}

bool fremont::CheckTF2Installed(const std::filesystem::path& steamDir) {
  std::ifstream file(steamDir / std::filesystem::path("steamapps/libraryfolders.vdf"));
  if (!file.is_open()) {
    return false;
  }
  std::string line;
  while (getline(file, line))
    if (line.find("440") != std::string::npos) {
      A_printf("[Fremont] TF2 found!\n");
      return true;
    }
  return false;
}

bool fremont::CheckSDKInstalled(const std::filesystem::path& steamDir) {
  std::ifstream file(steamDir / std::filesystem::path("steamapps/libraryfolders.vdf"));
  if (!file.is_open()) {
    return false;
  }
  std::string line;
  while (getline(file, line))
    if (line.find("243750") != std::string::npos) {
      A_printf("[Fremont] SDK2013MP found!\n");
      return true;
    }
  return false;
}

void fremont::curl_callback(void* buffer, size_t n) {
  if(!bin)
    curl_string_data += (char*)buffer;
  else{
    curl_bin_data.insert(curl_bin_data.end(),(char*)buffer,(char*)buffer+n);
  }
}

size_t fremont::static_curl_callback(void* buffer, size_t sz, size_t n, void* cptr) {
  static_cast<fremont*>(cptr)->curl_callback(buffer, n);
  return sz*n;
}

std::string fremont::get_string_data_from_server(const std::string& url) {
  CURL* curlHandle = curl_easy_init();
  curl_string_data = "";
  curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, fremont::static_curl_callback);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);
  CURLcode res = curl_easy_perform(curlHandle);
  if (res != CURLE_OK) {
    exit(256);
  }
  curl_easy_cleanup(curlHandle);
  return curl_string_data;
}


#if _WIN32
inline bool Is64BitWindows()
{
#if _WIN64
    return true;
#else
    USHORT ProcessMachine;
    USHORT NativeMachine;
    BOOL IsWow64 = IsWow64Process2(GetCurrentProcess(), &ProcessMachine, &NativeMachine);

    if(IsWow64)
    {
        if(NativeMachine == IMAGE_FILE_MACHINE_AMD64) return true;
    }

    return false;
#endif
}
#endif

std::filesystem::path fremont::GetSteamPath()
{
#if _WIN32
	char valueData[MAX_PATH];
	DWORD valueLen = MAX_PATH;

	//Check if this is 64 bit or 32 bit process
	const char* subKey = Is64BitWindows() ? "SOFTWARE\\WOW6432Node\\Valve\\Steam" : "\\SOFTWARE\\Valve\\Steam";

    //Get steam install dir from registry
	RegGetValueA(HKEY_LOCAL_MACHINE, subKey, "InstallPath", RRF_RT_ANY, nullptr, &valueData, &valueLen);
    if (valueData[0] == 0)
    {
        //Registry key did not exist/had no value
        return std::filesystem::path();
    }

	return std::filesystem::path(valueData);
#else
    std::string home = getenv("HOME");
    auto path = std::filesystem::path(home + "/.local/share/Steam/");
    if(std::filesystem::exists(path)){
        return std::filesystem::canonical(path);
    }else{
        return std::filesystem::path("");
    }
#endif
}

std::string fremont::get_butler() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
  std::string url = server_url + "/butler.exe";
  std::string temp_path = std::filesystem::temp_directory_path() / "butler.exe";
#else
  std::string url = std::string(PRIMARY_URL) + "butler";
  std::string temp_path = std::filesystem::temp_directory_path() / "butler";
#endif
  return download_to_temp(url, temp_path);
}
int fremont::DesktopNotif(const std::string title, std::string desc) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#else
  notify_init("Adastral");
  NotifyNotification* notification = notify_notification_new(title.c_str(), desc.c_str(), nullptr);
  notify_notification_set_timeout(notification, 3000);
  notify_notification_show(notification, nullptr);
  g_object_unref(G_OBJECT(notification));
#endif
}

std:: string fremont::download_to_temp(std::string url, std::string name){
    std::string temp_path = std::filesystem::temp_directory_path() / name;
    auto fp = fopen(temp_path.c_str(),"wb");
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
    std::filesystem::permissions(temp_path,std::filesystem::perms::all);
    return temp_path;
}



std::vector<char> fremont::get_bin_data_from_server(const std::string& url) {
  bin = true;
  CURL* curlHandle = curl_easy_init();
  curl_bin_data.clear();
  curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, fremont::static_curl_callback);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);
  CURLcode res = curl_easy_perform(curlHandle);
  if (res != CURLE_OK) {
        exit(256);
  }
  curl_easy_cleanup(curlHandle);
  return curl_bin_data;
}


