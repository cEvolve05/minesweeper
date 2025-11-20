# Mine Sweeper

A simple, classic game in cpp & slint.

## Build

1. prepare [vcpkg](https://vcpkg.io/), [CMake](https://cmake.org/) and a fast internet
2. install install recommended extension in VSCode
3. open CMake panel, select correspond configure, then config
4. select minesweeper as build target and launch/debug target
5. open `Run And Debug` panel, run `Launch (Windows)`, then should show app

### Prepare Slint Prebuilt (Optional)

Prebuilt will accelerate building.

1. open [Github Releases - Slint](https://github.com/slint-ui/slint/releases), download the latest version, install
2. remember the install path, create file `CMakeUserPresets.json` and fill it with the following content:

    ```json
    {
        "version": 10,
        "configurePresets": [
            {
                "name": "msvc-x64-prefix",
                "inherits": "msvc-x64",
                "displayName": "Native - MSVC x64 (Cmake Prefix)",
                "cacheVariables": {
                    "CMAKE_PREFIX_PATH": "<your install path>"
                }
            }
        ]
    }
    ```

3. In Build step, select added preset

## Development

Recommended: VSCode + clangd

1. install recommended extension
2. navigate to .vscode/settings.json, uncomment or append commented configs to your user setting

Then, enjoy it.
