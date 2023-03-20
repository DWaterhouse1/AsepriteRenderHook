# AsepriteRenderHook

Interprocess application for aseprite to live render the results of normal mapped sprites. Uses a highly overblown Vulkan renderer, implemented mostly as an exercise.

### Dependencies

- Vulkan
- glfw3
- ImGui
- EnTT
- Boost asio
- websocketpp

The Lua client is a simple script to be installed in aseprite.

### Aseprite Render Hook

This is a simple websocket server, which listens for sprite data sent by the aseprite client. It consumes the Engine as a static library, and uses it to live render the image data received.

### Engine

A simple Vulkan based renderer, with EnTT for scripting logic and ImGui for UI.

### Usage

Load an aseprite project with two layers called "Normal" and "Diffuse". Run the server first, and then execute the Aseprite script. It will send the two layers locally to the server, which will open basic Lambert render of your sprite based on the provided normal map. Simple controls are available, and the aseprite client will send updated versions to the renderer whenever you make changes.
