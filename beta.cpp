#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <comdef.h>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")

std::vector<DWORD> getRunningProcesses() {
    DWORD processes[1024];
    DWORD needed;
    std::vector<DWORD> result;

    EnumProcesses(processes, sizeof(processes), &needed);
    DWORD count = needed / sizeof(DWORD);
    for (DWORD i = 0; i < count; ++i) {
        result.push_back(processes[i]);
    }

    return result;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool containsAdvertisement(const std::wstring& title) {
    static const std::vector<std::wstring> triggerWords = {
        L"Advertisement", L"advertisement", L"panther", L"Panther"
    };


    for (const auto& word : triggerWords) {
        if (title.find(word) != std::wstring::npos) {
            return true;
        }
    }

    return false;
}

std::vector<DWORD> containsSpotify;
std::vector<DWORD> containsSpotifyAd;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD windowPID = 0;
    GetWindowThreadProcessId(hwnd, &windowPID);

    for (int i = 0; i < containsSpotify.size(); ++i) {
        if (windowPID == containsSpotify[i]) {
            wchar_t title[256];
            GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
            if (wcslen(title) > 0) {
                std::wcout << L"[Spotify Window] PID: " << windowPID << L", Title: " << title << std::endl;

                if (containsAdvertisement(title)) {
                    containsSpotifyAd.push_back(windowPID);
                    std::wcout << L"[Ad Match Found] -> Muting Spotify" << std::endl;
                }
            }
        }
    }

    return TRUE;
}

int main() {
    std::vector<DWORD> pids = getRunningProcesses();
    for (int i = 0; i < pids.size(); i++) {
        HANDLE processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, pids[i]);
        char fileName[MAX_PATH];
        GetModuleFileNameExA(processHandle, NULL, fileName, sizeof(fileName));
        std::string stringFileName = { fileName };
        if (ends_with(stringFileName, "Spotify.exe") || ends_with(stringFileName, "spotify.exe")) {
            containsSpotify.push_back(pids[i]);
        }
    }

    CoInitialize(NULL);
    IMMDeviceEnumerator* enumerator = NULL;
    IMMDevice* device = NULL;
    IAudioSessionManager2* sessionManager = NULL;
    IAudioSessionEnumerator* sessionEnumerator = NULL;

    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&enumerator));
    enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device);
    device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&sessionManager);
    sessionManager->GetSessionEnumerator(&sessionEnumerator);

    while (true) {
        containsSpotifyAd.clear();
        EnumWindows(EnumWindowsProc, 0);

        int count = 0;
        sessionEnumerator->GetCount(&count);
        std::cout << "\n[Audio Sessions Found]: " << count << std::endl;

        for (int i = 0; i < count; ++i) {
            IAudioSessionControl* sessionControl = NULL;
            sessionEnumerator->GetSession(i, &sessionControl);
            IAudioSessionControl2* sessionControl2 = NULL;

            if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2))) {
                DWORD pid;
                sessionControl2->GetProcessId(&pid);
                std::cout << "[Session PID]: " << pid;

                bool isSpotify = false;
                for (auto spotifyPID : containsSpotify) {
                    if (pid == spotifyPID) {
                        isSpotify = true;
                        std::cout << " --> [Matched Spotify PID]";
                        ISimpleAudioVolume* volume = NULL;
                        sessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&volume);
                        if (volume) {
                            float volLevel = containsSpotifyAd.empty() ? 1.0f : 0.0f;
                            std::cout << " --> [Set Volume: " << (volLevel == 0.0f ? "MUTE" : "FULL") << "]" << std::endl;
                            volume->SetMasterVolume(volLevel, NULL);
                            volume->Release();
                        } else {
                            std::cout << " --> [Volume Interface NULL]" << std::endl;
                        }
                    }
                }
                if (!isSpotify) std::cout << std::endl;
                sessionControl2->Release();
            }
            sessionControl->Release();
        }
        Sleep(1000);


    }

    sessionEnumerator->Release();
    sessionManager->Release();
    device->Release();
    enumerator->Release();
    CoUninitialize();

    return 0;
}
