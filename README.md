<br />
<div align="center">
  <a href="https://github.com/synthous/ntshot">
    <img src="img/logo.PNG" alt="Logo" width="100" height="100">
  </a>

  <h3 align="center">ntshot 📷</h3>

  <p align="center">
    FOSS Screenshot app, only for windows
    <br />
  </p>
</div>

# What? 🤔

FOSS Screenshot app, only for windows. It's simple app when user press `PrintScreen` in `screenshots` folder dir with `ntshot.exe` and config.

# Install 

- go to releases
- install latest version
- unzip in any folder
- check config (optional)
- launch `ntshot.exe`

# Config 📝
```
// cfg.txt
hotkey=Print Screen
folder=screenshots
filename=%Y%m%d_%H%M%S.png
```

# Build from source 🛠️

- install Visual Studio 2022
- setting up it
- make new project
- copy files from repository/src to vs project
- choose Release/x64 in preference
- setup gdi+
- run build
- copy config file from README.md
- Done

# License
GNU General Public License v3.0 [View](LICENSE)
