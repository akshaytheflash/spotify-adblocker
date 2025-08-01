# spotify-adblocker

so spotify supposedly used to use a title containing "Advertisement" on the desktop app
now it has changed, they use the ad title or something (attached)

<img width="269" height="241" alt="image" src="https://github.com/user-attachments/assets/a4d5c702-c761-4ab3-a326-2dfd597ad7ca" />

So my logic to use the window text to block audio is not working, i might think of a fix later, for now i made it into an vector you can add advertisement title substrings to if you want 🤷‍♂️

for example for this ad you can add L"GIVA" and/or L"Jewellery"

this was too much code to not upload 👍

> logic is it iterates all processes, finds processes whose module's file path ends in spotify.exe

> stores the PIDs of such processes (all spotify process trees) in a vector

> checks if any of them contain the wide strings we talked about, if it does, puts it in a new vector (named containsSpotifyAd i think)

> keeps checking in a loop if containsSpotifyAd is empty, if it is, means no ad (any window with the bad strings) is playing, so boosts the volume to max

> if it is non empty, means one of the bad strnigs is playing, so it mutes the app specific audio (sets it to 0)

## how to compile from source
use the visual studio command prompt if you face any errors using gpp

<img width="287" height="81" alt="image" src="https://github.com/user-attachments/assets/332e8371-ff08-4b5d-ba65-a9c48f9c8670" />

the command is ```cl /EHsc beta.cpp /link ole32.lib oleaut32.lib psapi.lib```
