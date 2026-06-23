# 📑 Wayland GTK Layer Shell Todo List

A lightweight, sleek, and desktop-integrated todo list built using C, **GTK3**, and **gtk-layer-shell**. Designed specifically for Wayland compositors (such as Hyprland, Sway, or Wayfire) to sit beautifully alongside desktop bars like Waybar.

Features a clean, modern dark theme, inline dynamic task editing, flat file persistence, and smooth interaction mechanics.

---

## ✨ Features

*   **Wayland Native Integration:** Uses the `gtk-layer-shell` protocol to anchor itself natively above or below windows, perfectly fitting status bar configurations.
*   **Inline Editing:** Edit your tasks instantly directly inside the row—no separate prompt screens or clunky popups needed.
*   **Persistent File Storage:** Saves states on the fly directly to a flat plain-text format (`~/.wayland_todo_list`).
*   **Automatic Empty States:** Displays a clean-slate placeholder message when all tasks are complete or deleted.
*   **Highly Custom CSS Styles:** Bundles local application themes targeting custom margins, border curves, and smooth focus states.

---

## 📦 Dependencies

Before building, ensure you have the necessary development libraries and standard header tools installed for your specific Linux distribution:

### Arch Linux
```bash
sudo pacman -S gtk3 gtk-layer-shell pango glib2 pkg-config base-devel
```

## 🔨 Build & Installation

Follow these quick setup commands to clone and compile the source layout directly from your local terminal session:

1. **Clone the Repository**
   ```bash
   git clone [https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git](https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git)
   cd YOUR_REPO_NAME
```

Compile the Source Code
Run the terminal build step below, using pkg-config to pass compiler search headers and automated library mappings:

```bash
    gcc main.c -o wayland-todo $(pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0 pangocairo glib-2.0)
```