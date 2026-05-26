# 🖥️ UDP-KM: Windows to Linux Input Sharing

A lightweight, low-latency Software KM (Keyboard and Mouse) written entirely in C. 

While inspired by traditional KVMs, this is a small, personal project designed for home use and **does not include any "Video" sharing or streaming capabilities**. It strictly handles the seamless transfer of keyboard and mouse inputs across a local network.

This project is built for a very specific, unidirectional setup: **Windows always acts as the Server (Host) and Linux acts as the Client**. It is perfect for dual-PC desk setups where Windows is your main machine and Linux is your workstation, allowing you to use a single mouse and keyboard for both.

## ✨ Main Features

* **One-Way Architecture:** Exclusively captures hardware inputs on Windows and emulates them on Linux.
* **Low-Latency UDP Protocol:** Uses UDP packet sending instead of TCP. By sacrificing strict packet retransmission, it achieves absolute immediacy, resulting in zero-lag mouse movements and keystrokes.
* **Hyprland Compatible:** Bypasses modern Linux display server security restrictions by separating virtual devices.
* **Multi-Touch Gestures:** Binds the Windows mouse side buttons to inject `ABS_MT_SLOT` coordinates, emulating 3-finger swipes on Linux to dynamically switch workspaces.

## ⚙️ Technologies and APIs Used

The project is split into two components communicating over a Local Area Network (LAN):

1. **Server (Windows):** Uses the **Win32 API** (specifically `WH_MOUSE_LL` and `WH_KEYBOARD_LL` low-level hooks) to intercept hardware inputs before the OS processes them. Network communication is handled via `Winsock2`.
2. **Client (Linux):** Uses **`libevdev` and the `uinput`** kernel subsystem to create physical-like virtual devices (a mouse, a touchpad, and a keyboard). It reads the incoming UDP packets, decodes them, and injects the events directly into the system.

---

## 🛠️ Dependencies and Requirements

To compile and run this program, you will need:

### On Windows (Server)
* **Compiler:** GCC (MinGW-w64).
* **Libraries:** `ws2_32` (included by default in the Windows SDK for sockets).

### On Linux (Client)
* **Compiler:** GCC.
* **libevdev library:** You must install the development headers.
  * Arch Linux: `sudo pacman -S libevdev`
* **pkg-config:** To easily link the library during compilation.

---

## 🚀 Setup and Compilation

Before compiling, **you must configure the IP addresses**.
Open `UDPClient.c` and make sure `inet_addr` points to your Linux machine's local IP (or use `INADDR_ANY` to listen on all interfaces). Likewise, ensure the IP in the Windows server code (`UDPServer.c`) points to your Linux client.

### 1. Compile the Windows Server
Open your terminal in the project directory and compile by linking the socket library:

```bash
gcc UDPServer.c window.c -o UDPServer.exe -lws2_32
```

### 2. Compile the Linux Client
Open your terminal in the project directory and compile using `pkg-config`:

```bash
gcc UDPClient.c keymap.c devices.c -o UDPClient $(pkg-config --cflags --libs libevdev)
```

## 🎮 How to Use

### 1. Start the Windows Server:
Run the compiled executable on your Windows machine:

```bash
UDPServer.exe
```
(A console window will appear indicating the socket is ready).

### 2. Start the Linux Client:
The client needs root privileges to create virtual devices in `/dev/uinput`.

```bash
sudo ./UDPClient
```
(On the window console will appear the messages "Hello Server" and "Hello Client Sent")

### 3. Transition
- Move your mouse cursor all the way to the left edge of your Windows screen.
- An invisible topmost window will be created on Windows to "trap" your physical mouse and prevent accidental local clicks. You will instantly gain full control of your Linux machine.
- To return to Windows, simply move the mouse to the right edge of the Linux screen and local control will be restored on Windows.

## 📝 Additional Notes
* **Virtual Resolution & Scaling:** The project hardcodes a base transition resolution of 1600x900. This specific default was chosen as a 1.6 downscale factor from a primary 2560x1440 (1440p) monitor. You can tweak the `virtualX` and `virtualY` variables in `UDPServer.c` to match the native resolution or the specific scaling ratio of your actual monitors.
* **Network Security:** Because UDP packets are sent unencrypted, it is highly recommended to use this software only on trusted home Local Area Networks (LAN).
